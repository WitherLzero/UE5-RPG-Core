# Handoff: ARPGPlayerController 光标逻辑分离 — Aura 恢复指引

## Mission

Warrior(第三人称近战 ARPG)接入 RPGCore 时,发现 `ARPGPlayerController` 携带的
光标追踪/高亮逻辑是 Aura(俯视角)专属的,不适合本类型游戏。本次工作把光标逻辑
从基类分离为可挂载组件,使基类视角中立。

**接手本文档的人 = 未来回去处理 Aura 的人。** 本文给出恢复 Aura 高亮功能所需的
全部动作与背景。

## Repository

- RPGCore 子模块:`Plugins/RPGCore`(本仓库 = RPGCore,独立 repo)
- 相关提交(按时间):
  - `f446bc4` `docs(adr)` ADR-0003 决策记录
  - `87f562f` `refactor(player)` 基类清理 + 光标配置化
  - `6ae3c49` `feat(player)` 新增 UCursorHighlightComponent(amend 后)
- Aura 宿主项目:`E:\Projects\ue5\Aura`(RPGCore 指向旧 commit `277d0a3`,需
  `git submodule update --remote Plugins/RPGCore` 拉新,见下文)

## 变更摘要

### 1. `ARPGPlayerController`(基类)视角中立化

- **删除**:`CursorTrace()`、`UpdateMouse()`、`LastActor/ThisActor` 高亮状态、
  节流字段、`STAT_CursorTrace`。
- **删除的 public API**(破坏性):
  - `GetCursorHit(FHitResult&)` → 由组件 `GetHoveredEnemy()` 语义替代
  - `HitEnemyActor()` → 同上
- **新增**:5 个 `EditDefaultsOnly` 光标配置属性,BeginPlay 内联应用一次:

| 属性 | 默认 | 说明 |
|---|---|---|
| `bShowCursor` | true | 是否显示鼠标 |
| `MouseCursorType` | Default | 光标样式(EMouseCursor) |
| `bUseGameAndUIMode` | true | true→GameAndUI;false→GameOnly |
| `MouseLockMode` | DoNotLock | 锁鼠标行为 |
| `bHideCursorDuringCapture` | false | 捕获时是否隐藏光标 |

> 默认值与 Aura 原行为一致,所以 Aura **现有资产无需改任何默认值**。
> 无动态切换方法 —— 技能需要切光标时自行调引擎 API(机制随所有者走,见 ADR-0003)。

### 2. `UCursorHighlightComponent`(新,挂 Pawn)

路径:`Source/RPGCore/{Public,Private}/GameplayMechanics/Core/Components/CursorHighlightComponent.{h,cpp}`

- 自 Tick,节流追踪光标下 Actor(默认 30Hz,可配 `TraceRate`)
- `Player.Block.CursorTrace` Tag 门控(ASC 在 owner Pawn 上)
- `LastActor/ThisActor` 高亮状态机,驱动 `IEnemyInterface::Highlight/UnHighlight`
- 公开查询:`GetHoveredEnemy()`(返回 `IEnemyInterface*`)
- 文件内局部 stat `STAT_CursorHighlight`

## Aura 恢复步骤(回去处理时直接照做)

### Step 1 — 拉新 RPGCore 并编译

```bash
cd E:/Projects/ue5/Aura
git submodule update --remote Plugins/RPGCore   # 或 git -C Plugins/RPGCore pull
```

编译确认无错。Source 层无引用被破坏,此步应静默通过。

### Step 2 — 挂载组件(恢复高亮)

在 Aura 的玩家 Pawn 上挂载 `UCursorHighlightComponent`。Aura 无自己的 PC 子类
(直接用 `ARPGPlayerController`),因此组件应挂在 **Character** 上。任选其一:

- **AuraCharacter.h** 构造函数:
  ```cpp
  UCursorHighlightComponent* CursorHighlight = CreateDefaultSubobject<UCursorHighlightComponent>(TEXT("CursorHighlight"));
  ```
- 或 BP 里 Add Component(注意:CursorHighlight 类在 BP 中按
  `GameplayMechanics > Core > Components` 分类可搜到)。

组件 BeginPlay 会取 owner Pawn 的 controller 作为检测来源;仅本地控制端 tick。

### Step 3 — 验证

1. PIE 单人:移动鼠标到敌人身上,应出现高亮;移开应取消。
2. 施加 `Player.Block.CursorTrace` Tag 期间,高亮应被抑制(原 CursorTrace 门控保留)。
3. 光标/输入模式:BegainPlay 后光标可见、GameAndUI 模式(与旧 Aura 一致)。

### Step 4 — 蓝图引用清扫(唯一风险点)

Aura **蓝图资产**可能引用被删的 `GetCursorHit` / `HitEnemyActor`(Source 层
grep 不到蓝图内引用)。打开编辑器后全局搜这两个名字:

- 编辑菜单 → 蓝图依赖/引用检查,或直接 Ctrl+Shift+F 全局搜文本。
- 若命中:改为读组件 `GetHoveredEnemy()`(需先通过 `GetComponentByClass` /
  BP 节点获取该组件)。

## 设计背景(为什么要这么做)

决策记录见 RPGCore `docs/adr/0003-playercontroller-genre-neutrality.md`,要点:

- 业界无"按视角分子类化 PC"的先例(Lyra = 薄 PC + Pawn 组件 + 配置资产);
- 框架内先例 `ARPGTargetActor_Indicator` 早已把光标轮询放在机制所有者内,PC 仅被借用;
- 基类只留引擎约定的 PC 通用职责(输入链、ASC、伤害文字池、光标可见性配置);
- "光标驱动高亮"是 top-down 机制 → 组件化,谁要谁挂;
- `Player.Block.CursorTrace` GameplayTag 保留在 RPGCore(未删除)。

## 相关文件

- `Source/RPGCore/Public/RPGFramework/Player/RPGPlayerController.h`
- `Source/RPGCore/Private/RPGFramework/Player/RPGPlayerController.cpp`
- `Source/RPGCore/Public/GameplayMechanics/Core/Components/CursorHighlightComponent.h`
- `Source/RPGCore/Private/GameplayMechanics/Core/Components/CursorHighlightComponent.cpp`
- `docs/adr/0003-playercontroller-genre-neutrality.md`
- `Source/RPGCore/Public/RPGFramework/Types/RPGGameplayTags.h`(`Player_Block_CursorTrace`)
