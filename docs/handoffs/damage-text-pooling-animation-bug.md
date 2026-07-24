# DamageText 池化动画 Bug 调查档案

## 文档目的

记录从 Component Pooling 迁移到 Actor Pooling 后出现的 UMG 动画失效 Bug 的完整排查过程，包括已确认的事实、未验证的假设、以及未解明的矛盾点。供后续续调查或知识库归档使用。

## 涉及版本

- UE 5.7.4
- RPGCore 子模块
- Aura 项目

---

## 一、架构背景

### 方案 A：Component Pooling（旧）

- `NewObject<UDamageTextComponent>(OwnerController, DamageTextCompClass)`
- `RegisterComponent()` → 创建 `UWidgetComponent`
- `AttachToComponent(TargetCharacter->RootComponent)` → 再 `DetachFromComponent`
- `SetDamageText(Damage, bBlockedHit, bCriticalHit)` → BP 端播放动画
- 池化：PreWarm 时 `NewObject` + `SetActive(false)` + `SetHiddenInGame(true)`
- Acquire：`SetActive(true)` + `SetHiddenInGame(false)` + 重新 Attach

**已知问题**：`AController::AController()` 默认调 `SetHidden(true)`，导致 `UWidgetComponent::UpdateWidgetOnScreen()` 中 `!(GetOwner()->IsHidden())` 检查失败，文字不渲染。

**修复验证**：在 `ARPGPlayerController::BeginPlay()` 加 `SetHidden(false)` 后，**所有技能**（火球、电刑、debuff、AOE）均正常显示动画并释放。

**为什么叫停**：`SetHidden(false)` 被认为"不规范"，担心隐藏的 Actor 状态依赖。

### 方案 B：Actor Pooling（当前）

- `ADamageTextActor` 继承 `AActor`，`CreateDefaultSubobject<UWidgetComponent>`
- `SpawnActorDeferred` + `FinishSpawning` 创建，PreWarm 后 `Released()` 进池
- Acquire：`SetActorLocation(WorldLocation)` → `Acquired()` → `SetDamageText()`
- `Released()`：`SetActive(false)` + `SetHiddenInGame(true)` + `SetVisibility(false)`
- `Acquired()`：`SetActive(true)` + `SetHiddenInGame(false)` + `SetVisibility(true)`

---

## 二、Bug 症状

| 技能 | 触发机制 | 方案 A + SetHidden(false) | 方案 B（原版） | 方案 B + 蓝图 Delay 0.0 | 方案 B + C++ FTSTicker |
|------|---------|--------------------------|---------------|------------------------|----------------------|
| 火球（FireBolt） | 投射物 Overlap → 伤害 | ✅ | ✅ | ✅ | ✅ |
| 电刑（Electrocute） | Channeled / 周期性伤害 | ✅ | ❌ 文字不播动画 | ✅ | ✅ |
| Debuff（DOT） | 周期 GE Tick | ✅ | ❌ 文字不播动画 | ✅ | ✅ |
| 水晶（ArcaneShards） | Timer + 投射物 Overlap + WaitTargetData | ✅ | ❌ 文字不播动画 | ❌ | ✅ |

**文字出现但不播动画 = Animation 不执行 OnAnimationFinished → 不回池**

---

## 三、排查过程

### 3.1 排除了蓝图逻辑差异

通过 Blueprint Extractor 确认 `GA_ArcaneShards.applyDamageToSingleTarget` 和 `GA_FireBolt` 的伤害路径一致：`MakeDamageEffectParamsFromClassDefaults` → `MakeDamageEffectSpec` → `BP_ApplyGameplayEffectSpecToTarget（目标 ASC）`。所有技能最终在 `VitalAttributeSet::PostGameplayEffectExecute` 中统一走 `ShowFloatingText` → `PC->ShowDamageNumber`。

排除：蓝图侧逻辑差异。

### 3.2 排除了 PoolManager 逻辑差异

`UDamageTextPoolManager::Acquire` / `Release` 对所有技能无差别调用。

排除：池管理逻辑差异。

### 3.3 发现 ListenServer 的 Server/Client 差异

在 ListenServer 双人测试中：

| 玩家角色 | 症状 |
|---------|------|
| Server（Host） | 同样问题（火球正常、AOE 不播动画） |
| Client（远程加入） | **所有技能正常** |

这是关键线索：**Client 端天然无 Bug**。

### 3.4 蓝图 Delay 0.0 的局部有效

在 `BP_DamageTextActor` 的 `SetDamageText` 事件中，在 `PlayDamageText` 前加 `Delay 0.0`：

- 电刑、Debuff → ✅ 好了
- ArcaneShards → ❌ 仍然不播动画

### 3.5 C++ FTSTicker 0.0 的全局有效

在 `ARPGPlayerController::ShowDamageNumber_Implementation` 中，将整个 Acquire + SetDamageText 通过 `FTSTicker::AddTicker(0.0)` 延迟一帧：

- 所有技能 → ✅ 都正常了

同时也将 `ADamageTextActor::Acquired()` 改为只执行 `SetActive(true)`（不执行 `SetHiddenInGame(false)`），将显示 widget 的时机推迟到 BP 中的 `SetDamageText` 事件内手动 `SetHiddenInGame(false)`，从而消除默认文本闪一下的副作用。

### 3.6 延时长度的行为差异

| 延时方案 | ArcaneShards 效果 | 副作用 |
|---------|------------------|--------|
| 不延时 | ❌ 不播动画 | 无 |
| 蓝图 Delay 0.0 | ❌ 不播动画 | 无 |
| 蓝图 Delay 0.1 ~ 0.2 | ✅ 正常 | PreWarm 的 10 个 Actor 依次闪烁默认文字 |
| C++ FTSTicker 0.0 | ✅ 正常 | 无 |

---

## 四、已知事实（Confirmed Facts）

1. **`UUMGSequenceTickManager::TickWidgetAnimations` 注册在 `FSlateApplication::OnPreTick()`**  
   来源：Zerol Dev Notes 源码分析。

2. **`FEngineLoop::Tick()` 中的执行顺序**（多个源码分析确认）：
   - `GEngine->Tick`（World Tick）→ `TickGroups` → `FTimerManager::Tick` → `ProcessLatentActions`
   - `FSlateApplication::Tick()` → `OnPreTick` → `TickApplication`（NativeTick, Paint）
   - `FTicker::GetCoreTicker().Tick`（可在 World/Slate 之后或之前，取决于具体引擎配置）

3. **Epic 官方承认该 Bug**（UE 5.5.4 论坛帖）：  
   "We addressed some similar issues with needing to wait a tick after construction before being able to play an animation."  
   修复合入：5.6 CL#41112585，额外修复 CL#43336467、CL#43410676。  
   用户环境：UE 5.7.4，**不确定上述修复是否完整覆盖此场景**。

4. **`PlayAnimation` 仅排队（Queue），不在当前帧立即评估**：  
   Epic 回复："PlayAnimation does indeed only queue the new animation, to be played later along with all the other ones queued this frame."

5. **Client 天然无 Bug**：`ShowDamageNumber` 是 `Client, Reliable` RPC。Client 执行 `_Implementation` 时天然跨了帧（网络延迟），时序上避开了问题窗口。

---

## 五、假设与未解明的问题

### 5.1 蓝图 Delay 0.0 为什么能修电刑/Debuff 但修不了 ArcaneShards？

这是核心未解问题。

**假说 A：技能触发时机在帧内的位置不同**
- 电刑/Debuff 的伤害走 `TG_PrePhysics`（TickGroups 内）
  - Acquire 在帧早期完成，Slate PreTick 在帧末尾 → 同帧处理动画 ✅
  - 蓝图 Delay 让 PlayAnimation 也在同帧 → 动画被 TickManager 正确看到
- ArcaneShards 的伤害走 `FTimerManager::Tick`
  - 此时 `ProcessLatentActions` 已过 → 蓝图 Delay 要等下一帧
  - Slate Tick 在帧末尾看到空壳 widget（无动画）→ 标记异常状态
  - 下一帧 PlayAnimation 补救时，TickManager 已错误标记 → ❌

**假说 B：WaitTargetData 后的 InputMode 切换导致 Slate 焦点状态过渡**
- 火球、电刑、Debuff 不需要 `WaitTargetData`，没有 InputMode 切换
- ArcaneShards 使用 `WaitTargetData` + `BP_CircleIndicator`，结束时 `SetInputMode(GameAndUI→GameOnly)`
- Slate 的 Input Focus 重建需要多帧稳定
- FTSTicker 的"一帧"（Slate Tick 之后）让 Focus 有充分时间稳定

**矛盾点**：如果假说 A 正确，蓝图 Delay 0.0 应该对 ArcaneShards 延迟到下一帧的 ProcessLatentActions，和电刑/Debuff 一样。如果假说 B 正确，蓝图 Delay 0.0 也延迟了一帧，应该也足够。两者都解释不了为什么蓝图 Delay 对 ArcaneShards 无效。

### 5.2 FTSTicker 为什么和蓝图 Delay 效果不同？

两者都是"等一帧再执行"。按帧序：

- **蓝图 Delay 0.0**：下一帧的 `ProcessLatentActions`（World Tick 内，在 Slate Tick 之前）
- **FTSTicker 0.0**：取决于 `FTicker::GetCoreTicker().Tick` 在帧序中的位置  

多个源码分析有分歧：
- SourcecodeReadingOfUnreal：FTSTicker 在 Slate Tick **之后**
- unreal_source_explained：FTSTicker 在 Slate Tick **之前**

如果 FTSTicker 在 Slate Tick 之后，就能解释为什么它更有效（Acquire 发生在 Slate 已经跑完的时间点，Widget 注册完全等下一轮 Slate 完整处理）。  
但这与另一个来源矛盾，需要用户在自己的 UE 5.7 环境中验证。

### 5.3 UUMGSequenceTickManager 的注册时机细节

- AddWidgetToScreen 是否同步更新 TickManager 的 widget 列表？
- IsConstructed() 的检测是否依赖 Slate Tick 的某个子阶段？
- 动画序列 Player 加入 ActiveSequencePlayers 后，TickManager 如何感知"这个 widget 有动画了"？
- 如果 widget 在 TickManager 列表中被标记为"无活跃动画"，PlayAnimation 能否正确唤醒它？

以上均需阅读 UE 5.7 源码确认，当前没有可靠来源。

### 5.4 MyGCWidget 的生命周期

- OnRegister 时 TakeWidget_Private 是否同步重建了 MyGCWidget？
- 如果 MyGCWidget 重建是延迟的，IsConstructed() 会在什么条件下返回 false？
- 在 Component Pooling 中 NewObject 组件和新 RegisterComponent 的流程和 CreateDefaultSubobject 的 OnRegister 流程有什么区别？

---

## 六、当前最终方案（已测试通过）

### C++ 改动

**DamageTextActor.cpp**：
```cpp
void ADamageTextActor::Acquired()
{
    // 只激活，不显示。显示推迟到 BP 的 SetDamageText 中。
    DamageTextComp->SetActive(true);
}

void ADamageTextActor::Released()
{
    DamageTextComp->SetActive(false);
    DamageTextComp->SetHiddenInGame(true);
}
```

**RPGPlayerController.cpp**：
```cpp
void ARPGPlayerController::ShowDamageNumber_Implementation(...)
{
    // 全局 1 帧延迟（FTSTicker 在 Slate Tick 之后执行）
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([...](float) {
            // Acquire + SetDamageText
            return false;
        }), 0.0f);
}
```

### BP 改动

`BP_DamageTextActor` EventGraph：
```
SetDamageText
  → Cast to WBP_DamageText
  → Get Damage Text Widget → Set Hidden In Game (false)  // 显示
  → PlayDamageText
  → Bind OnDamageTextFinished
```

不需要 Delay 节点。

---

## 七、参考资料

- [UE 5.5.4 论坛帖（最相关）](https://forums.unrealengine.com/t/5-5-4-unexpected-slate-animation-tick-behavior-affected-by-player-input/2601496)
- [UMG-Slate-Compendium](https://github.com/YawLighthouse/UMG-Slate-Compendium/blob/main/README.md)
- [Zerol Dev Notes：UUMGSequenceTickManager 分析](https://kisspread.github.io/notes/Basic/ECS/ecs.html)
- [SourcecodeReadingOfUnreal：FEngineLoop::Tick 帧序](https://sourcecodereadingofunreal.readthedocs.io/en/latest/content/Bootup.html)
- [unreal_source_explained：FEngineLoop::Tick 帧序](https://github.com/donaldwuid/unreal_source_explained/blob/master/main/loop.md)
- UWidgetComponent 源码（`OnRegister` / `OnUnregister` / `InitWidget` / `UpdateWidget` / `AddWidgetToScreen`）
- UUMGSequenceTickManager 源码（`TickWidgetAnimations` / `OnWidgetTicked`）

---

## 八、后续深入方向

1. 在自己的 UE 5.7 环境中验证 `FTSTicker::Tick` 和 `FSlateApplication::Tick` 的实际执行顺序（插日志或断点）
2. 阅读 `UUMGSequenceTickManager::TickWidgetAnimations` 在 5.7 中的具体实现，确认并行模式下的 widget 状态追踪机制
3. 确认 5.6 CL#41112585、CL#43336467、CL#43410676 是否合入 5.7.4
4. 对比 `NewObject<UWidgetComponent>` + 首次 `RegisterComponent` 和 `CreateDefaultSubobject` + `OnRegister` 在 MyGCWidget 生命周期上的差异
5. 验证 `ShouldDrawWidget()` / `IsConstructed()` / `UUMGSequenceTickManager` 的 widget 过期机制是否是导致该 Bug 的决定性因素
