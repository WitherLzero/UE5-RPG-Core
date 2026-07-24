# Profiling Workflow

## Problem Statement

Aura 和 RPGCore 代码中目前没有性能分析标记（profiling markers），导致：

1. 跑 `stat game` / `stat unit` 时只能看到引擎模块的耗时，看不出自己的代码哪里慢
2. 遇到卡顿无法定位热点是 Tick、GAS、伤害计算还是 UI
3. 做优化前后没有量化数据确认效果

## Solution

在代码中插入 `SCOPE_CYCLE_COUNTER` 标记，按阶段逐步覆盖高频路径，通过 `stat game` 实时观察。

**不引入额外依赖，Shipping 配置零开销。**

### Stat Group 设计

| Stat Group | 所属项目 | 编辑器控制台命令 | 声明文件 |
|-----------|---------|----------------|---------|
| `STATGROUP_RPGCore` | RPGCore 插件 | `stat RPGCore` | `RPGCore/Source/RPGCore/Public/RPGFramework/Stats/RPGCoreStats.h` |
| `STATGROUP_Aura` | Aura 项目 | `stat Aura` | `Source/Aura/Public/AuraGame/Stats/AuraStats.h` |

### 命名规则

```
Group 内 Stat 名不加项目前缀，由 Group 区分归属
显示名用 PascalCase，宏名统一加 `STAT_` 前缀

示例：
  DECLARE_CYCLE_STAT(TEXT("CursorTrace"), STAT_CursorTrace, STATGROUP_RPGCore);
  DECLARE_CYCLE_STAT(TEXT("AuraCharacterTick"), STAT_AuraCharacterTick, STATGROUP_Aura);
```

---

## Prerequisites（执行前确认）

1. 编译 Development Editor 配置能通过
2. 知道如何创建并打开 C++ 类
3. 编辑器控制台基本操作（`` ` `` 键打开）

---

## Phase 0 — 基础设施

### RPGCore

新建 `Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Stats/RPGCoreStats.h`：

```cpp
#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("RPGCore"), STATGROUP_RPGCore, STATCAT_Advanced);
// Phase 1 Stat declarations will be added here
// Phase 2 Stat declarations will be added here
// Phase 3 Stat declarations will be added here
```

### Aura

新建 `Source/Aura/Public/AuraGame/Stats/AuraStats.h`：

```cpp
#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Aura"), STATGROUP_Aura, STATCAT_Advanced);
// Phase 1 Stat declarations will be added here
// Phase 2 Stat declarations will be added here
```

### Build.cs

RPGCore 的 `RPGCore.Build.cs` 应已包含 `"Engine"` 模块依赖（`Stats.h` 属于 Engine 模块）。确认：

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "GameplayAbilities",
    "GameplayTags",
    "GameplayTasks",
    "Niagara",
    "MotionWarping"
});
```

Aura 的 `Aura.Build.cs` 同理确认包含 `"Engine"`。

### ✅ 验证

编译 Development Editor，无编译错误即通过。

---

## Phase 1 — 三个 Tick 函数

### 添加 Stat 声明

**RPGCoreStats.h** — 追加到 `DECLARE_STATS_GROUP` 下方：

```cpp
DECLARE_CYCLE_STAT(TEXT("PlayerTick"),      STAT_PlayerTick,         STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("CursorTrace"),     STAT_CursorTrace,        STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("TargetIndicatorTick"), STAT_TargetIndicatorTick, STATGROUP_RPGCore);
```

**AuraStats.h** — 追加：

```cpp
DECLARE_CYCLE_STAT(TEXT("AuraCharacterTick"), STAT_AuraCharacterTick, STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("AutoRun"),           STAT_AutoRun,           STATGROUP_Aura);
```

### 插入 SCOPE_CYCLE_COUNTER

**文件 1: `RPGCore/.../RPGPlayerController.cpp`**

```cpp
#include "RPGFramework/Stats/RPGCoreStats.h"

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_PlayerTick);
    Super::PlayerTick(DeltaTime);
    CursorTrace();
}

void ARPGPlayerController::CursorTrace()
{
    SCOPE_CYCLE_COUNTER(STAT_CursorTrace);
    // ... existing code
}
```

**文件 2: `RPGCore/.../RPGTargetActor_Indicator.cpp`**

```cpp
#include "RPGFramework/Stats/RPGCoreStats.h"

void ARPGTargetActor_Indicator::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_TargetIndicatorTick);
    Super::Tick(DeltaTime);
    // ... existing code
}
```

**文件 3: `Aura/.../AuraCharacter.cpp`**

```cpp
#include "AuraGame/Stats/AuraStats.h"

void AAuraCharacter::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_AuraCharacterTick);
    Super::Tick(DeltaTime);
    // ... existing code
}

void AAuraCharacter::AutoRun()
{
    SCOPE_CYCLE_COUNTER(STAT_AutoRun);
    // ... existing code
}
```

### ✅ 验证

1. 编译 Development Editor
2. 启动 PIE
3. 打开控制台（`` ` `` 键），输入：
   ```
   stat RPGCore
   ```
   应该看到 PlayerTick、CursorTrace、TargetIndicatorTick 条目
4. 再输入：
   ```
   stat Aura
   ```
   应该看到 AuraCharacterTick、AutoRun 条目
5. 移动角色/鼠标，观察各标记的耗时 MS 和百分比变化

---

## Phase 2 — GAS 核心路径

### 添加 Stat 声明

**RPGCoreStats.h** — 追加：

```cpp
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagHeld"),    STAT_AbilityInputTagHeld,    STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagPressed"), STAT_AbilityInputTagPressed, STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagReleased"),STAT_AbilityInputTagReleased,STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("ProcessInputTag"),        STAT_ProcessInputTag,        STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("SetEffectProperties"),    STAT_SetEffectProperties,    STATGROUP_RPGCore);
```

**AuraStats.h** — 追加：

```cpp
DECLARE_CYCLE_STAT(TEXT("ExecCalcDamage"),   STAT_ExecCalcDamage,   STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("OnNativeInput"),    STAT_OnNativeInput,    STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("HoldToMove"),       STAT_HoldToMove,       STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("SetupNavPoints"),   STAT_SetupNavPoints,   STATGROUP_Aura);
```

### 插入 SCOPE_CYCLE_COUNTER

| 文件 | 函数 | 宏 |
|------|------|----|
| `RPGAbilitySystemComponent.cpp` | `AbilityInputTagHeld` | `STAT_AbilityInputTagHeld` |
| `RPGAbilitySystemComponent.cpp` | `AbilityInputTagPressed` | `STAT_AbilityInputTagPressed` |
| `RPGAbilitySystemComponent.cpp` | `AbilityInputTagReleased` | `STAT_AbilityInputTagReleased` |
| `RPGPlayerController.cpp` | `ProcessInputTag` | `STAT_ProcessInputTag` |
| `RPGAttributeSetBase.cpp` | `SetEffectProperties` | `STAT_SetEffectProperties` |
| `ExecCalc_Damage.cpp` | `Execute_Implementation` | `STAT_ExecCalcDamage` |
| `AuraCharacter.cpp` | `OnNativeInput_Implementation` | `STAT_OnNativeInput` |
| `AuraCharacter.cpp` | `HoldToMove` | `STAT_HoldToMove` |
| `AuraCharacter.cpp` | `SetupNavPoints` | `STAT_SetupNavPoints` |

每个函数加一行，和 Phase 1 相同模式：

```cpp
void URPGAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
    SCOPE_CYCLE_COUNTER(STAT_AbilityInputTagHeld);
    // ... existing code
}
```

### ✅ 验证

1. 编译 → PIE
2. 同时开启两个组：
   ```
   stat RPGCore
   stat Aura
   ```
3. 打怪、放技能、按方向移动、切换目标
4. 观察各标记的耗时占比，特别关注：
   - `ExecCalcDamage` — 每次伤害触发，预期是最高的之一
   - `AbilityInputTagHeld` — 按住技能键时每帧触发
   - `SetEffectProperties` — 每次 GE 应用触发

---

## Phase 3 — 投射物 + 属性系统

### 添加 Stat 声明

**RPGCoreStats.h** — 追加：

```cpp
DECLARE_CYCLE_STAT(TEXT("ProjectileOverlap"),    STAT_ProjectileOverlap,     STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("HandleOnHit"),          STAT_HandleOnHit,           STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("OnHomingTrackerTick"),  STAT_OnHomingTrackerTick,   STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("VitalPostGEExecute"),   STAT_VitalPostGEExecute,    STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("RPGPostGEExecute"),     STAT_RPGPostGEExecute,      STATGROUP_RPGCore);
```

### 插入 SCOPE_CYCLE_COUNTER

| 文件 | 函数 | 宏 |
|------|------|----|
| `RPGProjectile.cpp` | `OnSphereOverlap` | `STAT_ProjectileOverlap` |
| `RPGProjectile.cpp` | `HandleOnHit` | `STAT_HandleOnHit` |
| `RPGProjectile.cpp` | `OnHomingTrackerTick` | `STAT_OnHomingTrackerTick` |
| `VitalAttributeSet.cpp` | `PostGameplayEffectExecute` | `STAT_VitalPostGEExecute` |
| `RPGAttributeSetBase.cpp` | `PostGameplayEffectExecute` | `STAT_RPGPostGEExecute` |

### ✅ 验证

1. 编译 → PIE
2. 使用投射物技能（FireBolt 等），向多个敌人发射
3. 观察 `ProjectileOverlap` / `HandleOnHit` 在命中瞬间出现
4. 使用制导投射物（多枚同时飞行），观察 `OnHomingTrackerTick` 在飞行期间的耗时
5. 观察 `VitalPostGEExecute` 和 `RPGPostGEExecute` 在每次掉血/加血时的触发

---

## Phase 4（可选）— UI 属性回调

如后续发现 UI 更新有明显卡顿，考虑在以下位置加标记：

- `OverlayWidgetController` 中的属性变化 lambda
- `AttributeMenuWidgetController::BindAttributeChangeDelegates`

届时再讨论具体实现。

---

## 附录 A — 编辑器控制台速查

| 命令 | 效果 |
|------|------|
| `stat RPGCore` | 开关 RPGCore 组 |
| `stat Aura` | 开关 Aura 组 |
| `stat game` | 显示游戏整体线程耗时 |
| `stat unit` | 显示帧耗时构成（Game/Render/GPU） |
| `stat cyclecounter` | 显示所有自定义 Stat 标记 |
| `stat -reset`或 `stat none` | 关闭所有 Stat 面板 |

操作提示：
1. `` ` `` 键打开控制台
2. 输入命令回车
3. Stat 面板通常出现在屏幕左上角
4. 面板中 MS = 每帧耗时（毫秒），% = 占 Game 线程百分比
5. 数值越高表示该函数越值得优化

## 附录 B — 操作流程速查

```
Phase 0: 建 2 个 Stats.h → 确认 Build.cs 有 Engine → 编译通过
   │
   ▼
Phase 1: 声明 5 个 Stat → 插入 5 个 SCOPE_CYCLE_COUNTER → 编译 → PIE → stat RPGCore & stat Aura 观察
   │
   ▼
Phase 2: 追加 9 个 Stat → 插入 9 个标记 → 编译 → PIE → 战斗中观察
   │
   ▼
Phase 3: 追加 5 个 Stat → 插入 5 个标记 → 编译 → PIE → 多投射物场景观察
   │
   ▼
Phase 4: 按需
```

每个 Phase 独立可做，可以在任意 Phase 停下，不影响游戏功能。
