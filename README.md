# UE5-RPG-Core

English | 简体中文

基于 Unreal Engine 5 Gameplay Ability System (GAS) 的轻量级 RPG 核心框架，面向 ARPG / 动作类 / GAS 驱动的 RPG 项目。框架提供 **基础设施层** 与 **机制复用层** 两层架构：底层处理 GAS 初始化、输入派发、属性/Tag 体系、AI 桥接、事件通信；上层提供投射物、指示器、战斗组件等可直接组合的机制。设计师可在不写 C++ 的前提下完成 80% 以上玩法搭建，**但关键生成逻辑（投射物生成、属性初始化、AI 控制器绑定）必须由游戏项目在 Blueprint 或 C++ 子类中实现**。

---

## 1. 快速开始

### 三步搭建第一个技能

| 步骤 | 做什么 | 去哪看 |
|------|--------|--------|
| **① 集成框架** | 将 RPGCore 添加到你的 UE5 项目并编译 | → §2 下方「集成指南」 |
| **② 按管线创建技能** | 依次创建 InputAction、DA_InputConfig、GE_Cost/Cooldown、Montage、GA_FireBolt、BP_FireBolt | → **§5.2 技能施放管线**（含完整资产表 + 字段示例值） |
| **③ 绑定按键 → 运行** | 在 DA_InputConfig 中建立 IA_FireBolt → Inputs.1 映射，按 1 键发射 | → §5.2② 管线流程 |

> **也可以直接从 §5 读起**：每条管线都从策划可编辑的资产出发，逐步深入到蓝图节点和 C++ 扩展点，找到你的需求对应的管线即可。

---

### 适用范围

| 维度 | 说明 |
|------|------|
| **适用类型** | ARPG、动作 RPG、GAS 驱动的技能系统项目 |
| **设计师可直接编辑** | 技能逻辑、属性数值、伤害公式、冷却/消耗、投射物参数、指示器样式、AI 反应 Tag、升级曲线、输入绑定 |
| **需要代码介入** | 新增 DamageType、自定义 TargetActor、复杂投射物运动逻辑、新 GameplayCue 类型、网络复制优化、底层 GAS 扩展、**投射物生成调用、属性集初始化、AI 控制器初始化** |
| **核心依赖架构** | `URPGAbilitySystemComponent` 统管能力/输入/委托 → `URPGGameplayAbilityBase` 为所有技能基类 → DataAsset / CurveTable 驱动数值 → Tag 体系贯穿 AI/特效/音频 |

---

## 2. 两层架构说明

```
RPGCore/
├── RPGFramework/          # 基础设施层：GAS/输入/属性/Tag/AI桥接/事件通信
│   ├── URPGAbilitySystemComponent
│   ├── URPGGameplayAbilityBase
│   ├── URPGInputConfig / URPGInputComponent
│   ├── RPGAttributeSetBase / VitalAttributeSet
│   ├── FRPGGameplayTags (struct)
│   ├── ARPGAIController
│   └── URPGFrameworkSettings
│
└── GameplayMechanics/     # 机制复用层：投射物、指示器、战斗组件
    ├── URPGProjectileSpell (stub ActivateAbility)
    ├── ARPGProjectile
    ├── ARPGTargetActor_Indicator
    ├── URPGAbilitySystemLibrary (SpawnProjectile helpers)
    ├── UDamageGameplayAbility
    └── CombatComponent / ICombatInterface
```

**关键边界**：
- `RPGFramework` 类大多为**完整实现**，直接使用或少量重写。
- `GameplayMechanics` 类多为**基类/工具**，核心生成逻辑（`ActivateAbility`、投射物 Spawn、TargetData 解析）留空，**游戏项目必须在子类中实现**。
- Aura 等游戏项目通过继承 `ARPGCharacterBase`、`ARPGPlayerState`、`ARPGAIController` 并重写 `InitAbilityActorInfo`、`InitDefaultAttributes`、`AddCharacterAbilities`、`InitAIwithASC` 完成接线。

---

### 集成指南

```
集成前提：UE 5.3+ C++ 项目，已启用 GameplayAbilities / EnhancedInput / Niagara / MotionWarping 插件。
```

RPGCore 以 **UE5 插件** 的形式分发，内含两个子模块：
- **RPGCore**（Runtime）— 运行时 RPG 框架代码
- **RPGCoreEditor**（Editor）— 编辑器工具（GameplayTag Browser 等）

**Step 1 — 将 RPGCore 作为子模块添加到项目 Plugins 目录**

推荐使用 git 子模块方式集成，便于后续更新：

```bash
# 在项目根目录下执行
git submodule add https://github.com/WitherLzero/UE5-RPG-Core.git Plugins/RPGCore
```

如果不需要 git 追踪，也可直接克隆：

```bash
git clone https://github.com/WitherLzero/UE5-RPG-Core.git Plugins/RPGCore
```

目录结构应为：

```
MyGame/
├── MyGame.uproject
└── Plugins/
    └── RPGCore/             # 本框架（UE5 插件）
        ├── RPGCore.uplugin  # 插件描述文件
        ├── Source/
        │   ├── RPGCore/     # Runtime 模块
        │   │   ├── RPGCore.Build.cs
        │   │   ├── Public/
        │   │   └── Private/
        │   └── RPGCoreEditor/  # Editor 模块
        │       ├── RPGCoreEditor.Build.cs
        │       ├── Public/
        │       └── Private/
        └── Content/         # 插件自带蓝图资产
```

> 插件作为 UE5 Plugin 自动发现，**无需**手动修改 `.uproject` 的 `Modules` 数组或 `Target.cs`。

**Step 2 — 在 `.uproject` 中注册插件**

打开 `MyGame.uproject`，在 `Plugins` 数组中添加 RPGCore：

```json
"Plugins": [
  { "Name": "RPGCore", "Enabled": true },
  { "Name": "GameplayAbilities", "Enabled": true },
  { "Name": "MotionWarping",     "Enabled": true },
  { "Name": "Niagara",           "Enabled": true }
]
```

> `EnhancedInput` 在 UE 5.3+ 中默认启用，无需单独配置。

**Step 3 — 在游戏模块 Build.cs 中添加依赖**

在 `Source/MyGame/MyGame.Build.cs` 中：

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "GameplayAbilities", "RPGCore"
});
```

**Step 4 — 编译**

使用 Rider / Visual Studio 编译 Development Editor，或右键 `.uproject` → Generate Project Files 后编译。

**Step 5 — 配置全局 DataAsset + 开始搭建**

1. 启动编辑器，在 Project Settings → RPG Framework 中指定三个全局 DataAsset：  
   `DA_AbilityInfo`、`DA_AttributeInfo`、`DA_LevelUpInfo`。
2. 从 Section 3 任务速查表找到你想做的功能，跳转到对应管线的详细说明。

---

> **快捷提示**：插件集成后，后续更新 RPGCore 只需 `git pull`（子模块方式则为 `git submodule update --remote Plugins/RPGCore`）并重新编译即可。Editor 工具（**Tools → RPGCore → GameplayTag Browser**）和编辑器蓝图资产随插件自动可用，无需额外配置。

---

## 3. 任务速查表

| 任务 | 去哪里改 | 关键资产/类 | 深入阅读 |
|-----------|----------|-------------|----------|
| 新增一个主动技能 | 继承 `RPGProjectileSpell` 或 `UDamageGameplayAbility` → 创建 `GA_XXX` | `GA_FireBolt`、`UDamageGameplayAbility`、`URPGGameplayAbilityBase` | 4.1 |
| 把技能绑定到键位 | 编辑 `DA_AuraInputConfig`，映射 `InputAction` → `GameplayTag` | `URPGInputConfig`、`URPGInputComponent` | 4.2 |
| 新增属性（如 Mana、Rage） | 创建 AttributeSet 子类 + 配置 `DA_AttributeInfo` | `RPGAttributeSetBase`、`VitalAttributeSet`、`UAttributeInfo` | 4.4 |
| 调整伤害公式 | 修改 `CT_Damage` CurveTable，或在 `UDamageGameplayAbility` 覆盖数值计算 | `UDamageGameplayAbility::DamageEffectParams`、`FScalableFloat` | 4.3 |
| 制作投射物技能 | 继承 `RPGProjectileSpell`，**在子类 Blueprint/C++ 中实现 Spawn 调用**，配置 `BP_Projectile` | `URPGProjectileSpell`、`ARPGProjectile`、`URPGAbilitySystemLibrary` | 4.1、4.5 |
| 制作地面指向性 AOE | 继承 `ARPGTargetActor_Indicator`，配置 `BP_CircleIndicator`，**在 Ability 中解析 TargetData 并调用生成逻辑** | `ARPGTargetActor_Indicator`、`AbilityTask_WaitTargetData` | 4.3、4.6 |
| 让 AI 对眩晕/击退做反应 | 在 `BT_EnemyBehaviorTree` 增加分支，**在敌人 Pawn/Character 中显式调用 `InitAIwithASC`** | `ARPGAIController`、`FRPGGameplayTags` (struct) | 4.2、4.7 |
| 配置升级经验曲线 | 编辑 `DA_LevelUpInfo` 中绑定的 CurveTable | `ULevelConfig`、`URPGFrameworkSettings` | 4.4 |

---

## 4. 命名与资产管理约定

统一的资产命名和目录结构能显著降低团队沟通成本，也是 RPGCore 项目推荐遵循的约定。

### 4.1 前缀约定

| 资产类型 | 前缀 | 示例 |
|----------|------|------|
| Gameplay Ability | `GA_` | `GA_FireBolt`、`GA_ArcaneShards` |
| Gameplay Effect | `GE_` | `GE_Cost_FireBolt`、`GE_Damage_Fire` |
| Blueprint 子类 | `BP_` | `BP_FireBolt`、`BP_CircleIndicator` |
| DataAsset | `DA_` | `DA_AbilityInfo`、`DA_AuraInputConfig`、`DA_LevelUpInfo` |
| CurveTable | `CT_` | `CT_Cost`、`CT_Damage`、`CT_Cooldown` |
| Input Action | `IA_` | `IA_LMB`、`IA_1`、`IA_AuraMove` |
| Behavior Tree | `BT_` | `BT_EnemyBehaviorTree` |
| Blackboard | `BB_` | `BB_EnemyBlackBoard` |

### 4.2 目录组织建议

```
Content/
├── Blueprints/
│   ├── AbilitySystem/
│   │   ├── Data/            # DA_AbilityInfo、DA_AttributeInfo、DA_LevelUpInfo
│   │   ├── Player/          # GA_Player 主动技能
│   │   ├── Enemy/           # GA_Enemy 技能
│   │   └── TargetActor/     # BP_CircleIndicator 等
│   ├── Input/               # DA_AuraInputConfig、IA_*
│   ├── AI/                  # BT_、BB_、BP_EnemyAIController
│   └── Characters/          # BP_Aura、BP_EnemyBase
```

### 4.3 GameplayTag 层级约定

| 层级 | 用途 | 示例 |
|------|------|------|
| `Abilities.{Element}.{Name}` | 具体技能 | `Abilities.Fire.FireBolt` |
| `Abilities.Type.{Type}` | 技能分类 | `Abilities.Type.Offensive`、`Abilities.Type.Passive` |
| `Abilities.Status.{Status}` | 技能状态 | `Abilities.Status.Locked`、`Abilities.Status.Equipped` |
| `Attributes.Vital.{Name}` | 生命法力 | `Attributes.Vital.Health`、`Attributes.Vital.Mana` |
| `Attributes.Meta.{Name}` | 元属性 | `Attributes.Meta.IncomingDamage` |
| `State.{Name}` | 角色状态 | `State.Stunned`、`State.Action.Targeting` |
| `Effects.{Name}` | 临时效果 | `Effects.HitReact` |
| `Inputs.{Key}` | 输入绑定 | `Inputs.1`、`Inputs.LMB` |
| `Montage.Attack.{Part}` | 攻击部位 | `Montage.Attack.Weapon`、`Montage.Attack.LeftHand` |

### 4.4 版本控制建议

- `.uasset` 文件采用 Git LFS 管理，避免仓库膨胀。
- DataAsset 和 CurveTable 的修改建议单独提交，便于策划回滚数值。
- C++ 类名、Tag、Blackboard Key 变更属于破坏性修改，需同步更新文档和 Blueprint 引用。

---

## 5. 实战示例与核心管线

### 5.1 核心管线概览

RPGCore 将 ARPG 中高频出现的玩法流程抽象为 **可配置管线（Pipeline）**。每条管线都遵循统一的阶段划分，并在每个阶段明确：RPGCore 提供什么、UE 原生 GAS 提供什么、业务项目需要填什么。

当前版本包含四条核心管线：

| 管线 | 解决的问题 | 典型场景 |
|---|---|---|
| **技能施放管线** | 输入 → 触发 → 瞄准 → 动画 → 效果 | FireBolt、近战技能 |
| **伤害结算管线** | 命中 → 计算 → 应用 → 反馈 | ExecCalc、受击反应 |
| **成长与技能配置管线** | 经验 → 升级 → 技能点 → 解锁/装备 | SpellMenu、AbilityInfo 状态 |
| **UI 数据管线** | ASC 状态变化 → WidgetController 委托 → UI 更新 | 血条、技能球、冷却遮罩 |

四条管线的协作关系如下：

```
Input Action
    ↓
InputTag (DA_AuraInputConfig)
    ↓
GameplayAbility (GA_FireBolt)
    ↓
DamageEffectParams (FDamageEffectParams)
    ↓
ExecutionCalculation (ExecCalc_Damage)
    ↓
IncomingDamage Meta Attribute → VitalAttributeSet
    ↓
GameplayCue / GameplayTag Event (Effects.HitReact)
    ↓
AI Blackboard  /  WidgetController Delegate  /  UI Update
```


这套管线的实际作用：

- **需求可拆解**：新增一个技能时，只需要改动对应管线阶段，不会触及全局架构。
- **责任可界定**：程序负责管线的“结构与扩展点”，策划负责管线中的“资产与数值”。
- **结构可叙述**：按管线阶段描述实现方式，比罗列蓝图节点更清晰。

每条管线的文档都按 `① 基础资产需求 → ② 管线流程 → ③ 蓝图节点参考 → ④ 程序扩展点` 组织：

- **策划 / 技术策划** 关注 **①**（创建哪些资产、填什么字段）和 **②**（数据如何流转），读完即可开始在编辑器中搭建。
- **程序开发者** 关注 **③**（RPGCore 提供了哪些可调用的节点/委托）和 **④**（C++ 需要实现的部分）。

---

### 5.2 管线一：技能施放（FireBolt）

以下示例以 FireBolt 展示如何利用 RPGCore 创建投射物技能：
策划在编辑器中继承框架提供的基类来创建蓝图资产，按本节流程完成配置；
程序根据代码扩展点在 C++ 中完成编写。

> **输入派发机制**：玩家按键 → `InputAction` → `RPGInputComponent` 查 `DA_InputConfig` 得到 `InputTag` → `ASC.AbilityInputTagPressed(InputTag)` → 匹配 `StartupInputTag` 相同的 `GA_XXX` → `TryActivateAbility()`。策划只需配置 `DA_InputConfig` 的映射表（见下方资产表中 `DA_PlayerInputConfig` 行），程序不介入。

---

#### ① 基础资产需求

| 资产                                    | 继承基类                                    | 用途描述                          | 关键配置字段（含示例值）                                                                                                                                                                                                                                                                                     |
| ------------------------------------- | --------------------------------------- | ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `InputAction` / `InputMappingContext` | Enhanced Input 原生资产                     | 将按键映射为输入事件                    | `IA_FireBolt`（绑定 Keyboard 1）                                                                                                                                                                                                                                                                     |
| `DA_PlayerInputConfig`                | `URPGInputConfig`（RPGCore DataAsset）    | 将 InputAction 关联到 GameplayTag | `TaggedInputActions[0]` → `InputAction: IA_FireBolt`, `InputTag: Inputs.1`                                                                                                                                                                                                                       |
| `GE_Cost_FireBolt`                    | `UGameplayEffect`（UE 原生 GameplayEffect） | 定义技能的法力消耗                     | `Modifiers[0]` → `Attribute: Mana`, `Op: Add`, `Magnitude: ScalableFloat(-20)`；或绑定 `CT_Cost` CurveTable 实现等级缩放                                                                                                                                                                                   |
| `GE_Cooldown_FireBolt`                | `UGameplayEffect`（UE 原生 GameplayEffect） | 定义技能的冷却时长与冷却标记                | `Duration Policy: Has Duration`；`DurationMagnitude: ScalableFloat(3s)` 或绑定 `CT_Cooldown`；`GrantedTags: Cooldown.FireBolt`                                                                                                                                                                        |
| `AM_Cast_FireBolt`                    | Animation Montage（UE 原生资产类型）            | 播放施法动画，在指定帧触发投射物生成            | 拖入角色施法动画片段；在出手帧添加 `Montage.Attack` Notify；可选在转身帧添加 `WarpingTarget` Notify                                                                                                                                                                                                                        |
| `GA_FireBolt`                         | `URPGProjectileSpell`（RPGCore 技能基类） | 技能核心——定义触发条件、瞄准方式、投射物生成参数 | `StartupInputTag: Inputs.1`<br> `CostGameplayEffectClass` → `GE_Cost_FireBolt`<br> `CooldownGameplayEffectClass` → `GE_Cooldown_FireBolt`<br> `DamageEffectClass` → `GE_Damage_FireBolt`<br> `DamageTypes: {Damage.Fire: 25 @ Lv1}`<br> `ProjectileClass` → `BP_FireBolt`<br> `NumProjectiles: 1`（Lv1）→ `3`（Lv5）<br> 可选：`DebuffChance(20)/Damage(5)/Duration(5)/Frequency(1)` |
| `BP_FireBolt`                         | `ARPGProjectile`（RPGCore 投射物 Actor 基类） | 投射物的外观、运动、碰撞与命中效果 | `ProjectileMovement` 子组件 → `InitialSpeed: 1500`, `MaxSpeed: 1500`<br> `Sphere` 子组件 → `SphereRadius: 按模型调整`, `Collision Preset: Projectile`<br> `ImpactEffect` → 命中 Niagara 系统<br> `ImpactSound` → 命中音效<br> `LoopingSound` → 飞行循环音效（可选）<br> `LifeTime: 5s` |

---

#### ② 管线流程（资产级）

```
[Input]
  IA_FireBolt (InputAction) + IMC (InputMappingContext)
  → 按常规配置项目所需的 Enhanced Input 资产
  → DA_PlayerInputConfig 中建立 IA → InputTag 映射
  → 放于 BP_PlayerController（继承 ARPGPlayerController）
      │
      ▼
[GA_FireBolt CDO]
  StartupInputTag: Inputs.1
  Cost/Cooldown/DamageEffectClass, DamageTypes, ProjectileClass
      │ 按键触发
      ▼
[Blueprint 参考连线逻辑]
  ① TargetDataUnderMouse → 获取鼠标位置
  ② K2_CommitAbility → 提交消耗/冷却
  ③ PlayMontageAndWait → 播放施法动画 + 发出 Warping 事件
  ④ WaitGameplayEvent (Montage.Attack) → 等待出手帧
  ⑤ GetCombatSocketLocation → 获取武器发射点
  ⑥ EvenlySpacedRotators → 计算散射方向
  ⑦ SpawnProjectileInDirection → 生成投射物
      │
      ▼
[BP_FireBolt 飞行]
  ProjectileMovement 驱动 → Overlap 检测碰撞
      │ 命中
      ▼
[→ 转到 5.3 伤害结算管线]
```

图中每个环节都对应你在基础资产需求表中创建的一个或一组资产。箭头表示数据流向，不涉及 C++ 内部实现。

---

#### ③ 技能设计相关蓝图节点参考

以下节点由 RPGCore 提供，按功能子组组织，在技能 Blueprint Event Graph 中可直接调用。

**瞄准与目标获取**

| 节点 | 效果 |
|------|------|
| `CreateTargetDataUnderMouse`（AbilityTask） | 捕获鼠标位置并转为 TargetData |
| `RPGTargetActor_Indicator`（TargetActor 类） | 地面 AOE 指示器 Actor，含 `DecalComp` 贴花组件（BlueprintReadWrite），配合 `WaitTargetData` 使用实现范围瞄准 |
| `GetLivePlayersWithinRadius` | 获取球形半径内所有存活 Actor |
| `GetClosestTargetsMax` / `GetClosestTargets` | 按距离排序获取最近 N 个 / 全部目标 |
| `TraceAttackTrajectory` | 从武器 Socket 到目标位置做碰撞检测 |
| `SendWarpingTargetEvent` | 发送 GameplayEvent 更新 Motion Warping 目标位置 |

**投射物生成**

| 节点 | 效果 |
|------|------|
| `SpawnProjectileTowardsTarget` | 生成向目标位置飞行的投射物 |
| `SpawnProjectileInDirection` | 生成沿指定方向飞行的投射物 |
| `SetHomingTargetActor` | 将追踪目标设为指定 Actor |
| `SetHomingTargetLocation` | 将追踪目标设为世界位置 |
| `SetHomingEnabled` | 启用/禁用追踪（含最小/最大加速度参数） |

**战斗 / 武器 / 蒙太奇**

| 节点 | 效果 |
|------|------|
| `GetWeapon` | 获取角色武器 SkeletalMesh |
| `GetCombatSocketLocation` | 根据 MontageTag 获取武器发射点的世界位置 |
| `GetAttackMontages` | 获取角色攻击蒙太奇数组 |
| `GetHitReactMontage` | 获取受击动画蒙太奇 |
| `PickRandomTaggedMontage` | 从蒙太奇数组中随机选取一个 |

**伤害

| 节点                                        | 效果                                                     |
| ----------------------------------------- | ------------------------------------------------------ |
| `MakeDamageEffectParamsFromClassDefaults` | 从 Ability CDO 默认值构建 `FDamageEffectParams`              |
| `MakeDamageEffectParamsFromCustomValues`  | 用显式传入的值构建 `FDamageEffectParams`（覆盖默认）                  |
| `MakeDamageEffectSpec`                    | 将 `FDamageEffectParams` 转为 `FGameplayEffectSpecHandle` |
| `GetDamageFalloff`                        | 根据距离计算径向伤害衰减系数                                         |

**数学工具**

| 节点 | 效果 |
|------|------|
| `EvenlySpacedRotators` | 在给定散射角内生成 N 个均分 Rotator（用于多弹幕散射） |
| `EvenlySpacedVectors` | 在给定散射角内生成 N 个均分方向向量 |
| `GetGroundRadialPoints` | 在地面平面生成圆形阵列点（用于地面 AOE 指示器布局） |

---

#### ④ 程序扩展点

以下是在 RPGCore 基础上设计一个技能时，可能需要程序端新增的代码。按"项目级配置 → 技能级实现"的顺序组织：

| 扩展点                  | 需要做什么                                                                                             | RPGCore 提供了什么                                                                                                                                                 | 备注                                              |
| -------------------- | ------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| **项目级 AttributeSet** | 继承 `URPGAttributeSetBase`，声明新属性的 UPROPERTY、ACCESSOR 宏、复制声明                                        | `URPGAttributeSetBase`（含 `TagsToAttributes` 映射、`PostGameplayEffectExecute`、`SetEffectProperties`）                                                             | 策划需要新属性维度时（如 Armor、Resistance）→ **详见 5.3 伤害管线** |
| **ExecCalc（伤害公式）**   | 继承 `UGameplayEffectExecutionCalculation`，按策划公式实现 `Execute`                                        | `FDamageEffectParams` 装配伤害参数、`FRPGGameplayEffectContext` 传递命中标记                                                                                               | 伤害计算逻辑由策划定公式程序写入 → **详见 5.3 伤害管线**              |
| **Combat 接口**        | 在项目 Character 类中实现 `ICombatInterface` 的 7 个蓝图事件，可自定义实现或将查询逻辑转接到 `UCombatComponent`                | `ICombatInterface`（`GetWeapon`、`GetCombatSocketLocation`、`GetAttackMontages`、`GetHitReactMontage` 等蓝图事件）、`UCombatComponent`（内含 WeaponMesh 注册 + 蒙太奇/Socket 路由） | 任意涉及武器/Socket/蒙太奇的技能都需要，项目完成一次后复用               |
| **GameplayTag 注册**   | 新增业务级静态 GameplayTag 结构体（如 `FMyGameplayTags`），在 `StartupModule` 中调用 `InitializeNativeGameplayTags` | `FRPGGameplayTags` 为参考模式，提供 DamageType、State、Effects、Cooldown、Input 等 Tag 分类范例                                                                                | 新增 DamageType 标签、技能状态标签、Buff/Debuff 标签时         |
| **新增蓝图节点**           | 在现有 Ability 父类添加 `UFUNCTION(BlueprintCallable/Pure)`，或新建 `UBlueprintFunctionLibrary`              | `URPGAbilitySystemLibrary`（35+ 个 Blueprint 节点）为参考模式                                                                                                           | 技能需要框架未提供的节点辅助逻辑时，建议在业务层建自有函数库                  |
| **自定义 TargetActor**  | 继承 `ARPGTargetActor_Indicator`，重写 `StartTargeting` / `ConfirmTargetingAndContinue`                | `ARPGTargetActor_Indicator`（含 `DecalComp` 贴花组件、地面追踪 Tick）                                                                                                     | 技能需要非圆形瞄准模式时（扇形、矩形、链式锁定）                        |
| **自定义投射物行为**         | 继承 `ARPGProjectile`，重写 `HandleOnHit` 或扩展 `ProjectileMovementComponent`                            | `ARPGProjectile`（含 Homing 追踪机制、Overlap 碰撞检测、`DamageEffectParams` 的 ExposeOnSpawn、`ImpactEffect`/`ImpactSound`）                                                | 需要弹跳、穿透、分裂、抛物线等非标准运动时                           |

---

### 5.3 管线二：伤害结算与受击反馈

以下示例承接 5.2 FireBolt，展示命中目标后的完整伤害结算流程：从 Ability 构造伤害参数，到投射物碰撞、ExecCalc 运算、Meta Attribute 消费、最终扣血与受击反馈。

RPGCore 核心层负责数据传递与结构约定，不参与具体伤害公式；伤害公式由业务层（Aura 的 `ExecCalc_Damage`）插入，按项目自身属性实现 Armor、Block、Critical、Resistance 等计算。

---

#### ① 基础资产需求

| 资产                      | 继承基类                                      | 用途描述                                               | 关键配置字段                                                                                                                                                                             |
| ----------------------- | ----------------------------------------- | -------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `GE_Damage_XXX` | `UGameplayEffect`（UE 原生 GameplayEffect） | 技能的基础伤害 GE，命中后对目标应用 | `Executions[0]` → 选择 `ExecCalc_Damage` 类（或自定义 ExecCalc），不需要在 GE Modifiers 中配置 Attribute；ExecCalc 内部通过 `Spec.GetSetByCallerMagnitude(DamageTypeTag)` 读取 Ability 运行时写入的原始伤害值，经公式运算后通过 `AddOutputModifier(IncomingDamage)` 输出最终伤害 |
| `DebuffCarrier_XXX`（可选） | `UGameplayEffect`（UE 原生 GameplayEffect）   | 空壳载体 GE，运行时由 `HandleDebuff` 动态填充减益参数               | `Duration Policy: Has Duration`；`Period: 1s`；`GrantedTags: State_Stun`（或对应减益 Tag）                                                                                                  |
| `CurveTable_XXX`        | CurveTable（UE 原生曲线表）                      | 伤害值的等级缩放曲线                                         | 曲线行名对应 `DamageType` Tag（如 `Damage.Fire`），横轴为等级，纵轴为基础伤害                                                                                                                             |
| `GA_HitReact`           | `UGameplayAbility`（UE 原生 GameplayAbility） | 受击反应 Ability，由 `Effects_HitReact` Tag 触发播放 Montage | `StartupInputTag` 留空（非按键触发）；可在 Blueprint 中播放受击 Montage                                                                                                                             |

> `GE_Damage_XXX` 和 `DebuffCarrier_XXX` 在每个技能创建时按需配置；`GA_HitReact` 和曲线表通常项目全局统一配置一次。

---

#### ② 管线流程（资产级）

```
[Ability CDO 配置]                    ← GA_FireBolt
  DamageEffectClass → GE_Damage_FireBolt
  DamageTypes → {Damage.Fire: FScalableFloat}
  DebuffChance / Damage / Duration / Frequency
       │ 命中
       ▼
[Spec 构造 + SetByCaller 写入]
   MakeDamageEffectParams → MakeDamageEffectSpec
   按 DamageType 写入原始伤害值
       │
       ▼
[ExecCalc 运算]                       ← GE_Damage_FireBolt（Executions[0]）
   读取原始伤害 → 公式运算（抗性/护甲/暴击）
   → 输出最终伤害
       │
       ├── 非致命                  ── 致命
       ▼                            ▼
[扣血 + 受击反馈]              [死亡流程]
   Health 减少                       Die → Ragdoll
   → GA_HitReact 激活                → 清理 AI
   （受击 Montage + Tag）                 
       │
       ▼
[AI 响应]
   Blackboard: HitReacting / IsStunned
   → Behavior Tree 分支切换
```

> ExecCalc 阶段是可选的。如果技能不需要复杂公式（如陷阱 Overlap 直接写入固定伤害），GE 可通过 SetByCaller 直接将原始值写入 IncomingDamage，跳过 ExecCalc。下游的 `HandleIncomingDamage` 不感知输入端是谁。

---

#### ③ 伤害构建相关蓝图节点参考

以下节点与伤害结算流程直接相关，在技能 Blueprint Event Graph 中可用于构建和检查伤害参数。

**伤害参数构建**

| 节点 | 效果 |
|------|------|
| `MakeDamageEffectParamsFromClassDefaults` | 从 Ability CDO 默认值构建 `FDamageEffectParams` |
| `MakeDamageEffectParamsFromCustomValues` | 用显式传入的值构建 `FDamageEffectParams`（覆盖默认值） |

**Spec 与上下文处理**

| 节点 | 效果 |
|------|------|
| `MakeDamageEffectSpec` | 将 `FDamageEffectParams` 转为 `FGameplayEffectSpecHandle`，准备应用 |
| `GetDamageFalloff` | 根据距离计算径向伤害的衰减系数（Inner → Outer 插值） |

**命中效果**

| 节点 | 效果 |
|------|------|
| `SetDamageText`（BlueprintImplementableEvent） | 显示浮动伤害数字（支持普通/格挡/暴击三种样式） |

> `IsBlockedHit`、`IsCriticalHit` 等 EffectContext 查询节点在 ExecCalc 内部由 C++ 使用，技能蓝图事件图中不直接出现。

---

#### ④ 程序扩展点

以下是在项目中建立伤害管线时，可能需要程序端新增的代码。

| 扩展点 | 需要做什么 | RPGCore 提供了什么 | 备注 |
|--------|-----------|-------------------|------|
| **项目级 AttributeSet** | 继承 `URPGAttributeSetBase`，声明新属性的 UPROPERTY、ACCESSOR 宏和 `DOREPLIFETIME` | `URPGAttributeSetBase`（`PostGameplayEffectExecute`、`SetEffectProperties`、`TagsToAttributes` 映射）；`UVitalAttributeSet`（Health/Mana、IncomingDamage/IncomingXP 元属性管道） | 策划需要新属性维度时（如 Armor、BlockChance、Resistance），参考 Aura 的 `UCombatAttributeSet` 实现模式 |
| **ExecCalc（伤害公式）** | 继承 `UGameplayEffectExecutionCalculation`，在 `Execute_Implementation` 中按策划公式实现计算 | `FDamageEffectParams` 装配伤害参数；`FRPGGameplayEffectContext` 传递命中标记；`SetByCaller` + `IncomingDamage` Meta Attribute 构成上下游通道 | ExecCalc 是 GAS 中唯一可同时捕获源/目标双方 Attribute 的地方。参考 Aura 的 `UExecCalc_Damage`：读取 Resistance → 应用减免 → Block/Crit 判定 → 输出到 IncomingDamage |
| **GameplayTag 注册（DamageType）** | 在业务级静态 Tag 结构体（如 `FAuraGameplayTags`）中注册 `Damage.Fire` 等标签 | `FRPGGameplayTags` 提供 DamageType、State、Effects 等 Tag 分类参考；`InitializeNativeGameplayTags` 模式 | 新增伤害类型时，同步需要在 `DamageTypesToResistances` 映射中关联对应的抗性 Tag |
| **EffectContext 扩展**（少用） | 在 `FRPGGameplayEffectContext` 中添加新字段，同时在 `NetSerialize` 中写入序列化代码 | `FRPGGameplayEffectContext` 已包含 Block/Crit/Debuff/Radial 标记；`URPGAbilitySystemGlobals::AllocGameplayEffectContext` 全局接入模式 | 仅在需要新增命中结果维度时（如"吸收"、"反弹"标记），当前标记位已覆盖大部分需求 |

**验证方法**

- 命中敌人后，服务器端 `ExecCalc_Damage` 应执行并输出日志；目标 Health 按 Armor/Resistance/Critical 结果扣减。
- 客户端显示三种跳字：普通 / Blocked / Critical，分别对应 `FRPGGameplayEffectContext` 中的标记位。
- 非致命伤害 → 敌人播放 HitReact Montage，Blackboard `HitReacting` 短暂为 true。
- 对其他技能配置含 `State_Stunned` 的 Debuff GE → 命中后 Blackboard `IsStunned` 为 true，AI 停止巡逻。

**常见错误**

- Ability 构建 Spec 时未调用 `AssignTagSetByCallerMagnitude` 写入原始伤害值 → ExecCalc 调用 `GetSetByCallerMagnitude` 返回默认值（-1），伤害数据丢失。
- ExecCalc 未捕获目标 Resistance 属性 → 该 Resistance 类型无法生效。
- `FRPGGameplayEffectContext` 自定义字段未在 `NetSerialize` 中序列化 → 客户端跳字状态与服务端不一致。
- 未调用 `ARPGAIController::InitAIwithASC` → Tag 变化无法同步到 Blackboard。
- Blackboard Key 名拼写错误（`HitReacting` / `IsStunned`） → AI 不响应。
- Behavior Tree Decorator 使用 `Is Set` 而非判断值等于 `true` → Tag 移除后分支无法恢复。

---

### 5.4 管线三：成长与技能配置（SpellMenu）

以下展示玩家从击杀获得经验到升级、解锁/装备技能的完整流程。每个环节对应你在基础资产需求表中创建的资产或 Blueprint 操作，箭头表示数据流向。

---

#### ① 基础资产需求

| 资产 | 继承基类 | 用途描述 | 关键配置字段 |
|------|---------|---------|-------------|
| `DA_AbilityInfo` | `UAbilityInfo`（RPGCore DataAsset） | 所有能力的元数据表：标签、类型、图标、解锁等级、法术球材质 | `AbilityInfos[]` 数组，每条 `FRPGAbilityInfo` 包含 `AbilityTag`、`AbilityType`（Offensive/Passive）、`LevelRequirement`、`Icon`、`BackgroundMaterial`、`CooldownTag`、关联的 `Ability` 类 |
| `DA_LevelUpInfo`（推荐） | `ULevelConfig` 派生类（如 `UAuraLevelConfig`） | 每级所需经验、升级奖励的属性点/技能点 | `LevelUpInformation[]`：`LevelUpRequirement`、`AttributePointAward`、`SpellPointAward`（数组下标对应等级——Index 0 无意义，Index 1 = Lv1→Lv2） |
| `CharacterClassInfo`（可选） | `UDataAsset` | 角色职业相关的初始属性 GE、启动 Ability、XP 奖励曲线 | `DefaultAttributes[]`、`StartupAbilities[]`、`XPReward` 曲线表 |
| `WBP_SpellMenu` | `UserWidget` | SpellMenu 根容器：布局法术球树、装备槽、技能点/描述/按钮区域 | 子容器嵌套：放置 `WBP_OffensiveSpellTree`、`WBP_PassiveSpellTree`、`WBP_EquippedSpellRow`、描述文本、Spend/Equip 按钮 |
| `WBP_SpellGlobe_Button` | `UserWidget` | 单个法术球：显示技能图标/背景/锁定态，响应点击选择 | 内部变量：`AbilityTag`（GameplayTag）、`Selected`（bool）；`OnAbilityInfoGet` 委托绑定 → `ReceiveAbilityInfo` 按 StatusTag 切换视觉 |
| `WBP_EquippedSpellRow` + `WBP_EquippedRow_Button` | `UserWidget` | 装备槽行容器 + 单个装备槽位按钮，显示已装备技能图标 | `WBP_EquippedRow_Button` 内部变量：`InputTag`（GameplayTag）、`AbilityType`（GameplayTag）；`Button_Ring` OnClicked → `SpellRowGlobePressed(InputTag, AbilityType)` |
| `WBP_OffensiveSpellTree` / `WBP_PassiveSpellTree` | `UserWidget` | 进攻/被动技能树容器，生成对应 AbilityType 的 `WBP_SpellGlobe_Button` 列表 | 互斥逻辑：选中一树法术球时自动调用另一树 `DeselectAll()` |
| `BP_SpellMenuWidgetController` | `UAuraWidgetController`（需业务层 C++ 新建，继承自 RPGCore `UWidgetController`） | SpellMenu 与 ASC 之间的中介：广播能力信息、处理点击/消耗/装备事件 | 绑定 `OnAbilityInfoGet`/`OnSpellGlobeSelected`/`OnSpellPointsChanged`/`OnWaitingForEquip` 到对应 Widget |

> `DA_AbilityInfo` 通过四级数据管线（Config → `URPGFrameworkSettings` → `LoadSynchronous()`）加载。`DA_LevelUpInfo` 通常在 `ARPGPlayerState` 的 `LevelConfig` 属性中直接引用。以上 Widget 位于项目的 `Content/Blueprints/UI/SpellMenu/` 目录下。

---

#### ② 管线流程（资产级）

```
[击杀目标]
   GE_XP_Reward → 产出经验值
     │
     ▼
[等级计算]
   DA_LevelUpInfo → 判断能否升级
     │
     ├── 升级：
     │    技能点增加
     │    能力状态更新（DA_AbilityInfo 解锁条件判定）
     │    回满血蓝 + 升级特效/音效
     │
     └── 未升级：
          XP 条百分比刷新
     │
     ▼
[SpellMenu 操作流程]
   ① 打开 SpellMenu → Controller 广播所有能力信息（图标/状态/等级）
   ② 点击法术球 → 显示技能描述 + 启用 SpendPoint/Equip 按钮
   ③ 消耗技能点 → 解锁新能力 / 升级已有能力
   ④ 装备能力 → 选择一个装备槽 → 挂载到动作条
     │
     ▼
[UI 自动更新]
   法术球图标/背景根据状态切换
   动作栏加装新技能 / 清空旧槽位
   技能点数字刷新
```

---

#### ③ UI相关蓝图节点/委托参考

以下节点和委托由 RPGCore 提供，可在 SpellMenu 或 Overlay Widget 的 Blueprint Event Graph 中直接使用。

**能力信息查询**

| 节点 | 效果 |
|------|------|
| `GetDescriptionsByAbilityTag`（ASC 方法） | 按 AbilityTag 返回当前等级描述和下一级描述 |
| `GetLockedDescription`（Ability 基类） | 返回法术球锁定状态的占位文本（含等级需求） |
| `FindAbilityInfoForTag`（AbilityInfo DataAsset） | 按 AbilityTag 查找能力的完整元数据（图标、类型、冷却 Tag） |

**交互回调（RPGCore 提供的 BlueprintAssignable 委托）**

| 委托 | 所在类 | 签名 | 触发时机 |
|------|--------|------|---------|
| `OnAbilityInfoGet` | `UWidgetController`（RPGCore 基类） | `FRPGAbilityInfo` | 能力信息初次广播或状态变更 |
| `OnAbilityLevelChanged` | `URPGAbilitySystemComponent`（RPGCore） | 无参数 | 任意能力的等级+1 时 |

---

#### ④ 程序扩展点

| 扩展点                            | 需要做什么                                                                                                                                                                                                                                                                                                                                                                                                       | RPGCore 提供了什么                                                                                                                                                              | 备注                                                                                                                                                                                                       |
| ------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **等级配置**                       | 继承 `ULevelConfig`，实现 `FindLevelForXP` / `GetXPRequirement`                                                                                                                                                                                                                                                                                                                                                  | `ULevelConfig` 基类（3 个虚方法）；四级数据管线                                                                                                                                           | 项目只需填表+派生一次。`AuraLevelConfig` 使用 `FAuraLevelUpInfo` 数组，每级单独配置 `LevelUpRequirement`、`AttributePointAward`、`SpellPointAward`                                                                               |
| **经验产出**                       | 在 `Enemy` 死亡 / 任务完成时应用含 `IncomingXP` 的 GE                                                                                                                                                                                                                                                                                                                                                                   | `VitalAttributeSet` 的 `PostGameplayEffectExecute` 已处理 IncomingXP 分发；`IPlayerInterface::AddToXP` 由 Character 实现                                                             | IncomingXP 是 Meta Attribute，不需要 Modifier 配置，通过 SetByCaller 写入                                                                                                                                            |
| **经验消费**                       | 在 Character 中实现 `AddToXP`（等级计算+奖励分发+LevelUp 调用）                                                                                                                                                                                                                                                                                                                                                             | `IPlayerInterface` 声明 `AddToXP`；`ARPGPlayerState` 提供 `LevelUp` / `AddToLevel` / `AddToXP`                                                                                  | 业务层（Aura）的实现。框架保证数据存储和下游通知                                                                                                                                                                               |
| **Ability 描述文本**               | 在项目 Ability 基类中重写 `GetDescription` / `GetNextLevelDescription` / `GetLockedDescription`                                                                                                                                                                                                                                                                                                                     | `URPGGameplayAbilityBase` 声明了这三个 `UFUNCTION(BlueprintNativeEvent)`                                                                                                         | 默认返回空字符串。`GetLockedDescription` 的 `LevelRequirement` 参数由 `GetDescriptionsByAbilityTag` 从 `DA_AbilityInfo` 中获取                                                                                            |
| **SpellMenu WidgetController** | 在业务层 C++ 中新建 WidgetController 子类（如 `USpellMenuWidgetController`）：<br>① 继承 `UWidgetController`（RPGCore 基类）或其项目桥接类<br>② 声明 SpellMenu 专用的 `BlueprintAssignable` 委托（`OnSpellGlobeSelected`、`OnWaitingForEquip` 等）<br>③ 实现 `BroadcastInitialValues()` 和 `BindCallbacksToDependencies()`<br>④ 添加 BlueprintCallable 函数（`SpellGlobeSelected`、`SpendPointButtonPressed`、`EquipButtonPressed`、`SpellRowGlobePressed`） | `UWidgetController` 基类：`OnAbilityInfoGet` 委托、`FWidgetControllerParams` 注入、`OnInitializeStartupAbilities` 遍历广播、`BroadcastInitialValues`/`BindCallbacksToDependencies` 虚函数钩子 | 这是 5.4 管线中**最重要的 C++ 扩展点**。RPGCore 不提供 SpellMenu 专用 Controller，业务层必须新建。`UAuraWidgetController`（Aura 中间桥接类）添加了 `GetASC()`/`GetAuraPS()` 等类型化访问器，具体子类 `USpellMenuWidgetController` 再在其上添加 4 个委托和 5 个按钮处理函数 |
| **装备流程**                       | 无需额外实现（RPGCore 提供完整服务端逻辑）                                                                                                                                                                                                                                                                                                                                                                                   | `ServerEquipAbility`：槽位冲突处理、被动替换、输入 Tag 分配、Client RPC 同步                                                                                                                   | 如需限制装备规则（如特定类型只能装特定槽位），在 `SpellRowGlobePressed` 中增加条件判断                                                                                                                                                  |
| **升级视觉效果**                     | 在 Blueprint 中重写 `BP_OnLevelUpVisualsTriggered`                                                                                                                                                                                                                                                                                                                                                              | `MulticastLevelUpEffects`（NetMulticast RPC，所有客户端同步调用此 BlueprintImplementableEvent）                                                                                         | 升级粒子、音效、闪白等纯视觉效果，策划在 Blueprint 中实现                                                                                                                                                                       |

**验证方法**

- 击杀敌人后，服务器的 `AddToXP` 日志显示 XP 数值累计。
- 经验达到等级阈值后，`OnLevelChanged` 触发；SpellMenu 中之前锁定的法术球变为"可解锁"状态（Locked→Eligible）。
- 点击法术球 → SpendPoint → 状态从 Eligible 变为 Unlocked 或 Unlocked→Level+1；`OnSpellPointsChanged` 通知技能点扣除。
- 点击 Equip → 选择槽位 → 该技能出现在动作条；原槽位技能被替换回 Unlocked 状态。
- Offensive/Passive 两树互斥：选中一支的法术球时，另一支的选中态自动清除。
- 取消装备 → 原槽位自动清空图标（`ClearGlobe`），技能回到 Unlocked 状态。
- 客户端之间同步：主机解锁技能后，Join-in-progress 的客户端应看到相同的能力状态和装备槽位。

**常见错误**

- `DA_AbilityInfo` 的 `AbilityTag` 与 `GA_XXX` 的 `AbilityTags` 不匹配 → `GetSpecFromAbilityTag` 返回 nullptr，解锁/装备失效。
- `DA_LevelUpInfo` 数组下标从 1 开始（Index 0 无意义）→ `FindLevelForXP` 的边界条件如果写错索引，升级时无法正确奖励技能点。
- 服务端 `AddCharacterAbilities` 之后未调用 `UpdateAbilityStatuses(Level)` → 所有能力保持 Locked，SpellMenu 中看不到任何法术球。
- `OnRep_ActivateAbilities` 未重写（客户端 `bStartupAbilitiesGiven` 不同步）→ Join-in-progress 时 `OnAbilityGiven` 不触发，WidgetController 无法初始化技能列表。
- `ServerEquipAbility` 中被动能力被替换时未处理 `OnPassiveAbilityDeactivated` → 旧被动技能持续生效。
- SpellGlobeButton 的 `Destruct` 中未解绑 `OnAbilityInfoGet` / `OnSpellGlobeReassigned` → Widget 销毁后委托残留回调，可能触发已销毁对象。

---

### 5.5 管线四：UI 数据同步（Overlay 与 SpellGlobe）

Overlay 是游戏主界面的根容器（血条、法力球、技能球、经验条、提示消息），所有数据由 `OverlayWidgetController` 统一管理。该 Controller 监听 ASC 上的属性变化/能力状态变更，通过 `BlueprintAssignable` 委托广播给 Widget Blueprint。

管线核心链路：**ASC 属性/Tag 变化 → WidgetController 回调 → 委托广播 → Blueprint EventGraph 接收 → UMG 控件更新**。

---

#### ① 基础资产需求

| 资产 | 继承基类 | 用途描述 | 关键配置字段 |
|------|----------|----------|-------------|
| `BP_OverlayWidgetController` | `UOverlayWidgetController`（需业务层 C++ 新建，继承自 `UAuraWidgetController` → RPGCore `UWidgetController`） | Overlay 数据源，管理所有 HUD 更新委托 | `MessageWidgetDataTable`（消息提示的 DataTable 引用） |
| `WBP_Overlay` | `UUserWidgetBase`（RPGCore 提供） | Overlay 根容器，持有所子 Widget 引用并触发级联初始化 | 子 Widget 变量：`WBP_HealthManaSpells`、`WBP_XPBar`、`WBP_ValueGlobe` |
| `WBP_HealthManaSpells` | `UUserWidgetBase`（RPGCore 提供） | 血条、法力条、技能球槽位的组合容器 | 内部嵌套 `WBP_SpellGlobe`、`WBP_HealthProgressBar`、`WBP_ManaProgressBar` |
| `WBP_XPBar` | `UUserWidgetBase`（RPGCore 提供） | 经验值进度条 | 监听 `OnXPPercentChanged` 委托 |
| `WBP_ValueGlobe` | `UUserWidgetBase`（RPGCore 提供） | 生命/法力数值球（显示具体数字） | 监听 `OnHealthChanged` / `OnManaChanged` 委托 |
| `WBP_SpellGlobe` | `UUserWidgetBase`（RPGCore 提供） | 单个技能球，显示图标/冷却遮罩/等级 | 监听 `OnAbilityInfoGet` 委托（RPGCore 提供）；内部使用 `WaitForCooldownChange` 异步任务 |
| `WBP_EffectMessage` | `UUserWidgetBase`（RPGCore 提供） | 动态创建的消息提示（击杀、拾取、升级等） | 构造函数接收 `FUIWidgetRow`：`Message`（文字）、`Image`（图标） |
| `WBP_LevelUpMessage` | `UUserWidgetBase`（RPGCore 提供） | 升级时的弹出提示 | `Text_Level`（TextBlock，显示新等级） |
| `BP_AuraHUD` | `AHUD`（UE 原生） | HUD 的创建入口：实例化 WidgetController 和 Overlay Widget | 引用 `BP_OverlayWidgetController` 和 `WBP_Overlay` 类 |

> 上述 Widget Blueprint 全部继承 `UUserWidgetBase`（RPGCore 提供），该基类声明了 `OnWidgetControllerSet` 虚函数和 `SetWidgetController`。所有子 Widget 不需要额外 C++ 类，仅在 Blueprint EventGraph 中完成委托绑定和控件更新。

---

#### ② 管线流程（资产级）

**初始化流程**

```
[BP_AuraHUD: BeginPlay]
    │
    ├── ① Create BP_OverlayWidgetController
    │      (SetWidgetControllerParams: ASC/PC/PS)
    │
    ├── ② Create WBP_Overlay（AddToViewport）
    │
    └── ③ WBP_Overlay.AddToViewport()
           │
           ▼
    [WBP_Overlay: OnWidgetControllerSet]
           │
           ├── ④ DynamicCast → BP_OverlayWidgetController_C
           │         → 存入 "Overlay Widget Controller" 变量
           │
           └── ⑤ ExecutionSequence（5 条并行分支）
                  │
                  ├── then_0: (DynamicCast 完成后无额外操作)
                  │
                  ├── then_1: WBP_HealthManaSpells.SetWidgetController
                  │              → then: WBP_XPBar.SetWidgetController
                  │                → then: WBP_ValueGlobe.SetWidgetController
                  │
                  ├── then_2: (空)
                  │
                  ├── then_3: 绑定 OnMessageWidgetRowGet
                  │              → OnMessageWidgetRowGet_Event:
                  │                BreakStruct(FUIWidgetRow)
                  │                → CreateWidget(WBP_EffectMessage)
                  │                → DynamicCast → WBP_EffectMessage_C
                  │                → SetTextAndImage(Message, Image)
                  │                → AddToViewport
                  │                → SetAlignmentInViewport(0.5, 0.5)
                  │                → SetPositionInViewport(Width*0.5, Height*0.75)
                  │
                  └── then_4: 绑定 OnPlayerLevelChanged
                               → OnPlayerLevelChangedDelegate_Event:
                                 IsValid(LevelUpMessage)?
                                   ├── Yes → RemoveFromParent（删除旧消息）
                                   └── No → CreateWidget(WBP_LevelUpMessage_C)
                                 → 存入 LevelUpMessage 变量
                                 → LevelUpMessage.Text_Level.SetText(NewValue)
                                 → AddToViewport
```

**运行时数据更新流程**

```
[ASC: GameplayAttribute 变化]
    │
    ▼
[OverlayWidgetController: BindCallbacksToDependencies]
    │  监听回调模板：OnHealthChanged、OnManaChanged、OnXPPercentChanged 等
    │
    ├── OnHealthChanged(float NewValue)         → WBP_HealthManaSpells / WBP_ValueGlobe
    ├── OnMaxHealthChanged(float NewValue)      → WBP_HealthManaSpells / WBP_ValueGlobe
    ├── OnManaChanged(float NewValue)           → WBP_HealthManaSpells / WBP_ValueGlobe
    ├── OnMaxManaChanged(float NewValue)        → WBP_HealthManaSpells / WBP_ValueGlobe
    ├── OnXPPercentChanged(float NewValue)      → WBP_XPBar
    ├── OnPlayerLevelChanged(float NewValue)    → WBP_Overlay → LevelUpMessage 创建
    └── OnMessageWidgetRowGet(FUIWidgetRow)     → WBP_Overlay → EffectMessage 创建
```

**技能球冷却流程**

```
[技能释放 → Cooldown Tag 添加]
    │
    ▼
[WBP_SpellGlobe: OnAbilityInfoGet_Event]
    │  ReceiveAbilityInfo(FRPGAbilityInfo)  ← 设置图标/背景
    │
    ├── IsValid(AsyncTask) → EndTask()     ← 清理前一次冷却任务
    │
    └── WaitForCooldownChange(ASC, CooldownTag)  ← RPGCore 异步任务
           │
           ├── CooldownStart(float TimeRemaining)
           │     → 存储 TimeRemaining
           │     → SetCooldownState()          ← 切换冷却遮罩显示
           │     → K2_SetTimerDelegate(UpdateTimer, Interval, Looping)
           │       → 存入 CooldownTimer 句柄
           │
           └── CooldownEnd
                 → SetDefaultState()           ← 恢复正常显示
                       → 清除定时器
```

---

#### ③ 蓝图节点参考（RPGCore 提供）

以下 BlueprintAssignable 委托和异步节点由 RPGCore 声明，UI Widget Blueprint 可直接绑定使用。

**WidgetController 基类委托**

| 节点/委托 | 所在类 | 签名 | 触发时机 |
|-----------|--------|------|---------|
| `OnAbilityInfoGet` | `UWidgetController`（RPGCore 基类） | `const FRPGAbilityInfo&` | 能力信息初次广播或状态变更 |

**异步任务节点**

| 节点 | 效果 |
|------|------|
| `WaitForCooldownChange`（`UWaitCooldownChange`） | 监听指定 CooldownTag 的添加/移除。`CooldownStart` 输出剩余时间，`CooldownEnd` 输出 0 |

**数据查询节点**

| 节点 | 效果 |
|------|------|
| `FindAbilityInfoForTag`（`UAbilityInfo`） | 按 `GameplayTag` 查询 `FRPGAbilityInfo` 结构（图标、类型、冷却 Tag、等级需求） |
| `GetDescriptionsByAbilityTag`（`URPGAbilitySystemComponent`） | 按 `AbilityTag` 获取能力的完整描述文本（等级需求、当前描述、下级描述） |

> 属性变化监听（`OnHealthChanged`、`OnManaChanged` 等）和消息提示委托（`OnMessageWidgetRowGet`）由业务层的 `UOverlayWidgetController` 声明——详见下方 ④ 程序扩展点。

---

#### ④ 程序扩展点

Overlay 管线中的 C++ 工作集中在 WidgetController 的创建和数据桥接上。RPGCore 提供了基类和异步任务框架，具体数据字段和委托声明属于业务层。

| 扩展点                          | 需要做什么                                                                                                                                                                                                                                                                                                                                                                                                                               | RPGCore 提供了什么                                                                                                                                                                     | 备注                                                                                                                                 |
| ---------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| **Overlay WidgetController** | 在业务层 C++ 中新建 `UOverlayWidgetController`：<br>① 继承 `UAuraWidgetController`（业务层桥接类 → RPGCore `UWidgetController`）<br>② 声明 5+ 数据委托：`OnHealthChanged`、`OnMaxHealthChanged`、`OnManaChanged`、`OnMaxManaChanged`、`OnXPPercentChanged`、`OnPlayerLevelChanged`、`OnMessageWidgetRowGet`<br>③ 实现 `BroadcastInitialValues()`：初次打开时广播当前属性/经验/等级值<br>④ 实现 `BindCallbacksToDependencies()`：注册 ASC 属性变化回调，将 `OnAttributeChangeData.NewValue` 转发到对应委托 | `UWidgetController` 基类（含 `FWidgetControllerParams` 注入、`BroadcastInitialValues`/`BindCallbacksToDependencies` 虚函数钩子）；`FOnPlayerInfoChangedSignature` 委托类型；`FOnAbilityInfoGet` 委托类型 | 这是 Overlay 管线**最重要的 C++ 扩展点**。每个 `DECLARE_DYNAMIC_MULTICAST_DELEGATE` + `UPROPERTY(BlueprintAssignable)` = 一个可在 Blueprint 中绑定的数据通道 |
| **消息提示 DataTable**           | 在业务层 C++ 中：<br>① 定义消息行结构体（如 `FUIWidgetRow`，含 `MessageTag`、`Message` 文本、`MessageWidget` 类引用、`Image` 图标）<br>② 继承 `UDataTable` 创建 `DT_MessageWidgetData`，行类型设为上述结构体<br>③ 在 `OverlayWidgetController` 中实现 Tag→Row 的查询逻辑（如 `GetDataTableRowByTag<T>`）                                                                                                                                                                                    | `UDataTable`（UE 原生）、`FGameplayTag` 查询基础设施                                                                                                                                         | 结构体定义和查询方法全部属于业务层 C++。策划拿到配置好的 DataTable 后，直接填 Tag/文字/图片即可                                                                         |
| **HUD 初始化**                  | 创建 `BP_AuraHUD`（继承 `AHUD`），在 Blueprint EventGraph 或 C++ 中：<br>① 创建 `BP_OverlayWidgetController` → 注入 `FWidgetControllerParams`<br>② 调用 `BroadcastInitialValues()`<br>③ 创建 `WBP_Overlay` → `SetWidgetController` → `AddToViewport`                                                                                                                                                                                                   | `UWidgetController` 的 `SetWidgetControllerParams` 函数                                                                                                                              | HUD 是纯业务层资产，RPGCore 不提供 HUD 基类                                                                                                     |

---

**验证方法**

- 进入游戏后，血条/法力球/经验条显示正确数值，且与 ASC 上的 `CurrentHealth`/`CurrentMana`/`XP` 属性一致。
- 受到伤害时血条实时减少；使用技能后法力球实时减少；击杀敌人后经验条逐步推进。
- 技能球显示正确的图标和背景（来自 `DA_AbilityInfo`）；释放技能后冷却遮罩出现并按预期倒计时。
- 升级时弹出 `WBP_LevelUpMessage`，显示新等级数字。
- 特定游戏事件（如击杀、拾取）触发对应 `WBP_EffectMessage`，显示正确消息文字和图标。
- 技能球 `Destruct` 后不再触发任何委托回调（`RemoveDelegate` 在 `Destruct` 中调用）。

**常见错误**

- `BP_OverlayWidgetController` 未在 `BP_AuraHUD` 中创建/注入参数 → `OnWidgetControllerSet` 不触发，所有子 Widget 无数据。
- `WBP_Overlay` 的 `then_1` 链未串联（`WBP_HealthManaSpells` → `WBP_XPBar` → `WBP_ValueGlobe`）→ 下游 Widget 从未收到 `SetWidgetController`，数据显示为空。
- `OnHealthChanged` 等属性委托的 `BroadcastInitialValues` 未调用 → Widget 首次打开时显示 0，等待下一次属性变化才更新。
- `WaitForCooldownChange` 的 `InCooldownTag` 与 `DA_AbilityInfo` 中配置的 `CooldownTag` 不匹配 → 冷却遮罩从不出现。
- `WBP_SpellGlobe` 未在 `Destruct` 中调用 `RemoveDelegate(OverlayWidgetController.OnAbilityInfoGet)` → Widget 销毁后仍在接收数据，可能触发已销毁对象访问。
- `OnMessageWidgetRowGet` 的 `FUIWidgetRow.Image` 字段未配置 → 效果消息 Widget 显示空白纹理。

---

### 5.6 模式替换指南：瞄准模式

以 Arcane Shards（地面 AOE）为例，展示如何将 5.2 FireBolt 的鼠标锁定瞄准替换为范围指示器模式。核心变化是用 `WaitTargetData` + 指示器 Actor 替代 `TargetDataUnderMouse`。

---

#### ① 额外资产需求

相比 5.2 FireBolt 的资产表，瞄准模式替换需要新增以下资产：

| 资产 | 继承基类 | 用途描述 | 关键配置字段 |
|------|---------|---------|-------------|
| `BP_ArcaneIndicator` | `ARPGTargetActor_Indicator`（RPGCore） | 地面范围指示器：半透明圆形贴花，随鼠标移动实时更新位置 | 拖入 Decal 材质；`SphereRadius` 决定范围大小；`Collision Preset: OverlapAll` |
| `GA_ArcaneShards` | `URPGProjectileSpell`（RPGCore 技能基类） | 与 `GA_FireBolt` 结构相同，但瞄准方式改为地面范围 | `TargetingType: GroundAOE`（需在 Blueprint 中实现）；`ProjectileClass: 按需`；其余配置参考 5.2 |

> 其余资产（InputAction、Cost/Cooldown GE、Animation Montage）的创建方式与 5.2 FireBolt 一致，不再重复。

---

#### ② 管线流程（资产级）

```
[GA_ArcaneShards: ActivateAbility]
    │
    ▼
[WaitTargetData] ← 指定 BP_ArcaneIndicator 类
    │  每帧更新指示器位置
    ▼
[BP_ArcaneIndicator: StartTargeting]
    │  生成 Decal Actor，跟随鼠标在地面移动
    │
    ├── 右键 → CancelTargeting → 技能结束
    │
    └── 左键 → ConfirmTargeting
              │
              ▼
        [TargetData: FHitResult]
              │  包含命中位置/法线
              ▼
        [解析 TargetData]
              │  GetHitLocation → 投射物生成点
              │  
              ▼
        [SpawnProjectileInDirection]
              或 ApplyEffectToTarget（无需投射物的地面 AOE）
```

> 注意区分：带投射物的 AOE（如 Arcane Shards 从天而降）走 SpawnProjectile → 投射物飞行 → Overlap 伤害；无投射物的 AOE（如地面火焰圈）直接 `ApplyGameplayEffectToTarget` 对范围内 Actor 应用 GE。

---

#### ③ 瞄准相关蓝图节点参考

以下节点由 RPGCore 提供，用于构建瞄准流程。**瞄准相关节点的完整列表已在 5.2③ 列出**，此处仅标注模式替换核心差异：

| 替换点 | FireBolt（5.2） | Arcane Shards（5.6） |
|--------|----------------|---------------------|
| 目标获取 | `CreateTargetDataUnderMouse` | `AbilityTask_WaitTargetData` + `ARPGTargetActor_Indicator` |
| 投射物生成 | `SpawnProjectileTowardsTarget`（向目标 Actor 飞行） | `SpawnProjectileInDirection`（向地面位置发射）或直接 `ApplyEffectToTarget` |
| 命中判定 | 投射物 Overlap 碰撞 | 指示器范围 + 投射物落地 Overlap |

---

#### ④ 程序扩展点

| 扩展点 | 需要做什么 | RPGCore 提供了什么 | 备注 |
|--------|-----------|-------------------|------|
| **自定义 TargetActor** | 继承 `ARPGTargetActor_Indicator`，重写 `StartTargeting` / `ConfirmTargetingAndContinue` | `ARPGTargetActor_Indicator`（含 Decal 组件、地面追踪 Tick） | 扇形/矩形/链式锁定等非圆形瞄准模式在此扩展 |
| **自定义投射物行为** | 继承 `ARPGProjectile`，重写运动逻辑或 `HandleOnHit` | 见 5.2④ 自定义投射物行为行 | 如 Arcane Shards 从天空落下可重写初始位置计算 |

> 这两个扩展点的详细说明已在 **5.2④** 中列出，此处不再重复。如有自定义瞄准需求，请先参考 5.2④ 的对应行。

---

#### 验证方法

- 进入游戏，装备 Arcane Shards 技能 → 按键后地面出现圆形指示器。
- 移动鼠标，指示器应实时跟随地面位置。
- 左键确认 → 指示器位置处产生投射物或 AOE 效果。
- 右键取消 → 技能结束，不消耗冷却/消耗。
- 取消后即可再次按键重新进入瞄准。

#### 常见错误

- `WaitTargetData` 的 `TargetActorClass` 未指定 `BP_ArcaneIndicator` → 指示器不生成，技能无反应。
- 指示器 Collision Preset 设为 `OverlapAll` 但不响应地面碰撞 → 使用 `ECC_GameTraceChannel1 (Ground)` 通道。
- `ConfirmTargeting` 后的 `TargetData` 未正确传送到 Spawn 逻辑 → 检查 `FHitResult` 的 `Location` 是否有效。
- 右键取消流程未调用 `CancelTargeting` → 技能状态卡在"瞄准中"，无法再次按键。

## 6. 子系统操作手册

### 6.1 GAS 初始化

**流程**：
- `ARPGCharacterBase::PossessedBy` / `OnRep_PlayerState` → `InitAbilityActorInfo(Owner, Avatar)`（**空实现，游戏项目 Character 子类必须重写**）
- `ARPGPlayerState::InitAbilityActorInfo` → 创建 ASC、注册属性集（**空实现，游戏项目 PlayerState 子类必须重写**）
- `URPGAbilitySystemComponent::AddCharacterAbilities` 遍历 `CharacterAbilities` 数组赋予 Startup Abilities（**空实现，游戏项目 ASC 子类或 Character 子类必须重写**）
- `ApplyEffectToSelf` 应用 `DefaultAttributes` GE（初始化生命值、法力值等）
- 属性变化 → ASC 委托广播 → UI / 任务系统监听更新

**可编辑资产**：`DA_AbilityInfo` (初始能力列表)、`DA_AttributeInfo` (属性显示名/描述)、`URPGFrameworkSettings` (全局默认 DataAsset)。

> 关键类（`URPGAbilitySystemComponent`、`ARPGCharacterBase`、`ARPGPlayerState`）已在 **Section 2 架构图**列出，此处不重复。


### 6.2 AI Tag 桥接
**定位**：将 ASC 上的 GameplayTag 变化实时同步到 AI Blackboard，驱动 Behavior Tree 状态切换。  
**可编辑资产**：`BP_EnemyAIController` 中的 Tag-Blackboard 映射、`BT_EnemyBehaviorTree` 的 Decorator/Service。  
**关键代码类**：`ARPGAIController`、`FRPGGameplayTags` (struct)、`UBlackboardComponent`。  
**数据流**：
- **游戏项目 Pawn/Character 显式调用** `ARPGAIController::InitAIwithASC(EnemyASC)` → 监听 ASC Tag 变化回调
- `OnHitReactTagChanged` / `OnStunTagChanged` → 将 Tag Count 写入 Blackboard Bool Key
- Behavior Tree Decorator 监听 Blackboard Key → 触发分支切换
- 常用映射：`State_Stunned → IsStunned`、`Effects_HitReact → HitReacting`（**无 `b` 前缀**）

### 6.3 跨系统事件通信

RPGCore 提供以下跨系统通信机制（UI 事件已在 **5.5 UI 数据同步管线**中详述）：

- **GameplayCue**：GE / Ability 指定 `GameplayCueTag` → `UGameplayCueManager` → 客户端播放 Niagara / Sound / Montage。新增自定义 Cue 类型 → 参见 Section 7 需求决策索引。
- **ASC 级委托**：`OnOutOfHealth`、`OnAbilityGiven`、`OnAbilityStatusChanged`、`OnAbilityEquipped` 等——供 Audio Manager、Quest / 成就系统等非 UI 模块监听。
- **Tag 变化**：配合 6.2 AI Tag 桥接，任何系统可通过 GameplayTag 堆叠/移除触发自定义逻辑。

---

## 7. 需求决策索引

本节回答"我有一个需求，谁来做、去哪里看"。按是否需要写代码分为三条路径：

- **框架契约** — RPGCore 设计上要求项目必须实现的步骤，可 Blueprint 或 C++
- **按需扩展** — 不属于框架契约的 C++ 工作，引入新机制时按需完成
- **策划自助** — 不写代码，直接编辑 DataAsset / CurveTable / Blueprint

每个需求标注了关联管线（→ 5.x）或子系统（→ 6.x）。

---

### 框架契约（Blueprint 或 C++，项目必须实现）

| 需求 | 关联位置 | 说明 |
|------|---------|------|
| 在 Character/PlayerState 子类中实现 GAS 初始化三件套 | → **6.1 GAS 初始化** | 重写 `InitAbilityActorInfo`、`InitDefaultAttributes`、`AddCharacterAbilities`。框架提供空壳，项目填充具体属性集和初始技能列表 |
| 在 RPGProjectileSpell 子类中实现投射物生成调用 | → **5.2 技能施放管线** | 重写 `ActivateAbility`，调用 `SpawnProjectileTowardsTarget` 或 `SpawnProjectileInDirection`。框架不自动生成投射物 |
| 在敌人 Pawn 中显式调用 InitAIwithASC | → **6.2 AI Tag 桥接** | `BeginPlay` 或 `PossessedBy` 中绑定 ASC 到 AIController。不调用则 AI 不响应 Tag 状态变化 |

> 这三项是 RPGCore 留给项目的启动接线步骤。创建新项目时优先检查它们是否已实现。

### 按需扩展（纯 C++，引入新机制时按需完成）

| 需求 | 关联位置 | 说明 |
|------|---------|------|
| 新增 GameplayCue 类型（如全屏后处理） | → **5.3 伤害管线 / 6.3 跨系统事件** | 继承 `UGameplayCueNotify_Static` / `Actor`，实现 `OnExecute`。按 UE GAS 原生方式扩展 |
| 网络优化：预测/回滚/相关性控制 | 全局架构（无固定管线） | 调整 `URPGAbilitySystemComponent` 复制策略、`NetExecutionPolicy`。属于底层调优 |

> 其余纯 C++ 扩展点（自定义 TargetActor、ExecCalc、GameplayTag 注册等）已在 **5.2④ / 5.3④** 对应管线中列出，此处不重复。

### 策划自助（无需写代码，直接编辑资产）

| 需求 | 关联位置 | 操作入口 |
|------|---------|---------|
| 调整技能冷却/消耗/伤害数值 | → **5.2 / 5.3** | 编辑 `CT_Cost`、`CT_Cooldown`、`CT_Damage` CurveTable |
| 新增一个普通投射物技能 | → **5.2** | 继承 `RPGProjectileSpell`，配置 Blueprint 字段；子类中实现 Spawn 调用 |
| 新增地面 AOE 指示器样式 | → **5.6 瞄准模式替换** | 继承 `ARPGTargetActor_Indicator`，替换 Decal 材质/大小；Ability 中解析 TargetData |
| 配置 AI 对新 Tag 的反应 | → **6.2 AI Tag 桥接** | 在 `BP_EnemyAIController` 增加 Tag→Blackboard 映射，BT 增加 Decorator 分支 |
| 修改升级经验/属性成长曲线 | → **5.4 成长管线** | 编辑 `DA_LevelUpInfo` 中绑定的 CurveTable |
| 更换技能图标/描述/最大等级 | → **5.4** | 编辑 `DA_AbilityInfo` 对应行 |

---

## 8. 调试与常见问题排查

### 8.1 常用控制台命令

| 命令 | 作用 |
|------|------|
| `showdebug abilitysystem` | 显示 ASC 当前激活的 Abilities、Effects、Attributes |
| `showdebug abilitysystem input` | 显示输入绑定和 Ability 匹配状态 |
| `stat gameplayabilities` | 查看 GAS 性能开销 |
| `ai.debug` / 按键 `'` | 打开 AI Debug，观察 Behavior Tree 和 Blackboard |
| `show collision` | 查看碰撞体，排查投射物/指示器无法命中问题 |

### 8.2 典型问题排查流程

**问题：按键后技能不触发**
1. 检查 `DA_AuraInputConfig` 中 `InputAction` → `InputTag` 映射是否存在。
2. 检查 `GA_XXX` 的 `StartupInputTag` 是否与 `DA_AuraInputConfig` 中的 `InputTag` 一致。 → 参见 5.2① 资产表
3. 检查 Character 的 `SetupPlayerInputComponent` 是否正确调用了 `BindTaggedAction`。
4. 检查 ASC 是否已初始化（`InitAbilityActorInfo` 是否被调用）。 → 参见 6.1 初始化流程

**问题：投射物不生成**
1. 检查 `GA_XXX` 是否继承 `RPGProjectileSpell` 或相关子类。
2. 检查 `ProjectileClass` 是否已设置。 → 参见 5.2① 资产表 `GA_FireBolt` 行
3. 检查 `ActivateAbility` 是否实现了 Spawn 调用（框架不自动生成）。 → 参见 7. 框架契约
4. 检查 `BP_Projectile` 是否开启 `bReplicates`。

**问题：AI 不响应 Stun/HitReact**
1. 检查敌人 Pawn 是否显式调用了 `InitAIwithASC`。 → 参见 7. 框架契约
2. 检查 Blackboard Key 名是否为 `IsStunned` / `HitReacting`（无 `b` 前缀）。 → 参见 6.2 数据流
3. 检查 Behavior Tree Decorator 是否判断键值等于 `true`（仅 `Is Set` 会在 Tag 移除后仍保持触发）。
4. 检查 `GA_HitReact` 内部的 GE 是否正确赋予 `Effects_HitReact` Tag（伤害 GE 本身不负责该 Tag）。 → 参见 5.3 常见错误

**问题：伤害不生效**
1. 检查 `DamageEffectParams.DamageGameplayEffectClass` 是否已设置。 → 参见 5.2① 资产表
2. 检查 `MakeDamageEffectSpec`（或自定义 Spec 构建）是否调用了 `AssignTagSetByCallerMagnitude` 传入原始伤害值。
3. 检查目标 ASC 是否注册了 `VitalAttributeSet`。 → 参见 6.1 初始化流程
4. 检查 `VitalAttributeSet::PostGameplayExecute` 是否正确处理了 `IncomingDamage`。 → 参见 5.3 常见错误

---

## 9. 相关文档

- **Epic GAS 官方文档**：Gameplay Ability System Overview（必备前置阅读）
- **API 参考**：计划输出至 `docs/API-Reference.md`（待补充）
