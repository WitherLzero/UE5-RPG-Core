# PRD: DamageTextComponent Pooling

## Problem Statement

当前 `RPGPlayerController::ShowDamageNumber_Implementation` 在**每次命中**时都会执行以下操作：

```cpp
UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextCompClass);
DamageText->RegisterComponent();
DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), ...);
DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
```

同时，通过蓝图阅读发现：

- `WBP_DamageText` 在 `Construct` 时播放 `DamageAnim` / `HitMessageAnim` 两个动画，动画总时长约 **1 秒**。
- 动画播完后，**没有任何节点销毁或释放该组件**。
- 组件会一直挂在 `TargetCharacter` 身上，直到目标角色本身被 `Destroy`。

这意味着每次命中都会：

1. 新建 `UDamageTextComponent` 对象（`NewObject` + `RegisterComponent`）。
2. 分配一个 `UUserWidget` 实例。
3. 在目标角色身上留下一个不可见但仍在 GC 引用链中的组件。

在 AOE / 高攻速 / 多段伤害场景下，会导致：

- 大量运行时 UObject 分配，触发 GC 尖刺。
- 每个敌人身上积累越来越多的残留组件，占用内存与 Tick/Update 开销。
- 蓝图中动画仅在 `Construct` 播放一次，无法复用同一个 Widget 实例。

## Solution

引入 `UDamageTextComponent` 对象池：

1. 在 `ARPGPlayerController` 中维护一个 `UDamageTextPoolManager` 对象。
2. 启动时 **PreWarm** 固定数量（默认 10）的 `UDamageTextComponent` 实例。
3. 命中时从池中 **Acquire** 一个组件，Attach 到目标角色，调用 `SetDamageText`。
4. 动画结束后，组件 **Detach** 并 **Hide**，归还到池中等待复用。
5. 如果池子耗尽，允许临时创建新组件，但动画结束后仍归还给池子，池子动态扩容。

通过把动画播放从 `Construct` 移到显式函数，并增加动画结束回调，让组件可以被完整复用。

---

## Current Flow (Before)

```
命中事件
  │
  ▼
RPGPlayerController::ShowDamageNumber_Implementation
  │
  ├── NewObject<UDamageTextComponent>(TargetCharacter)
  ├── RegisterComponent()
  ├── AttachToComponent(TargetCharacter->RootComponent)
  └── SetDamageText(Damage, bBlocked, bCritical)
        │
        ▼
  BP_DamageTextComp::SetDamageText
        │
        ├── GetUserWidgetObject
        ├── Cast to WBP_DamageText
        └── Call UpdateDamageText
              │
              ▼
        WBP_DamageText::UpdateDamageText
              │
              ├── Set Text_Damage
              ├── Set Text_HitMessage
              └── Set Color (based on Blocked/Crit)

WBP_DamageText::Construct
  │
  ├── PlayAnimation(DamageAnim)      ← 仅在新建时触发
  └── PlayAnimation(HitMessageAnim)    ← 仅在新建时触发

动画结束后：无操作，组件永久挂载
```

## New Flow (After)

### 1. 初始化

```
ARPGPlayerController::BeginPlay
  │
  ▼
创建 UDamageTextPoolManager
  │
  ▼
PreWarm(10)  ← 创建 10 个 BP_DamageTextComp，不 Attach、隐藏
```

### 2. 命中显示

```
命中事件
  │
  ▼
RPGPlayerController::ShowDamageNumber_Implementation
  │
  ▼
UDamageTextPoolManager::Acquire(TargetCharacter)
  │
  ├── 从 AvailablePool 找一个空闲组件
  │   ├── 找到 → Attach + Show
  │   └── 没找到 → NewObject 新建并加入池
  │
  ▼
返回 UDamageTextComponent*
  │
  ▼
SetDamageText(Damage, bBlocked, bCritical)
  │
  ▼
BP_DamageTextComp::SetDamageText
  │
  ├── GetUserWidgetObject
  ├── Cast to WBP_DamageText
  └── Call WBP_DamageText::PlayDamageText
        │
        ├── UpdateDamageText  ← 更新数字/颜色/文字
        ├── PlayAnimation(DamageAnim)
        └── PlayAnimation(HitMessageAnim)
  │
  ▼
Bind to WBP_DamageText::OnDamageTextFinished
```

### 3. 动画结束归还

```
WBP_DamageText::OnAnimationFinished
  │
  ▼
OnDamageTextFinished.Broadcast()   ← 自定义委托，WBP 不依赖外部类
  │
  ▼
BP_DamageTextComp::OnDamageTextFinished Event
  │
  ▼
UDamageTextComponent::ReturnToPool()
  │
  ▼
UDamageTextPoolManager::Release(Component)
  │
  ├── DetachFromComponent
  ├── SetHiddenInGame(true)
  ├── SetActive(false)
  └── 放回 AvailablePool
```

---

## Architecture Decisions

### 1. Pool Manager 是独立 UObject 类

新建 `Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Pooling/DamageTextPoolManager.h`

理由：

- 职责与 `ARPGPlayerController` 分离。
- 未来可在其他需要伤害数字的 Controller / ASC 中复用。
- 方便后续把投射物池化等通用逻辑统一放在 `RPGFramework/Pooling` 目录下。

```cpp
UCLASS()
class RPGCORE_API UDamageTextPoolManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(ARPGPlayerController* Owner, TSubclassOf<UDamageTextComponent> Class);
    void PreWarm(int32 Count);

    UDamageTextComponent* Acquire(ACharacter* AttachTarget);
    void Release(UDamageTextComponent* Component);

    void Shutdown();

private:
    UPROPERTY()
    TObjectPtr<ARPGPlayerController> OwnerController;

    UPROPERTY()
    TSubclassOf<UDamageTextComponent> DamageTextClass;

    UPROPERTY()
    TArray<TObjectPtr<UDamageTextComponent>> AvailablePool;

    UPROPERTY()
    TArray<TObjectPtr<UDamageTextComponent>> ActivePool;

    int32 MaxPoolSize = 32;
};
```

### 2. 归还入口放在 `UDamageTextComponent` 基类

在 `RPGCore/Source/RPGCore/Public/RPGFramework/UI/Widgets/DamageTextComponent.h` 中增加：

```cpp
class RPGCORE_API UDamageTextComponent : public UWidgetComponent
{
    // ... 现有代码 ...

public:
    UFUNCTION(BlueprintCallable)
    void ReturnToPool();

    UFUNCTION(BlueprintPure)
    bool IsInUse() const { return bInUse; }

    void SetInUse(bool bInUse);

protected:
    UPROPERTY()
    bool bInUse = false;
};
```

理由：

- `ReturnToPool()` 是组件级别的操作，由组件自己知道所属池（通过 `GetOuter()` 或 Manager 引用）。
- 避免 BP_TextComp 暴露内部池管理细节。
- `IsInUse` 标记用于快速判断组件是否可用。

### 3. 动画结束委托放在 `WBP_DamageText` 上

`WBP_DamageText` 增加自定义委托：

```cpp
UPROPERTY(BlueprintAssignable)
FOnDamageTextFinished OnDamageTextFinished;
```

理由：

- 动画结束是 Widget 自身的生命周期事件。
- WBP 不依赖 `UDamageTextComponent` 或 Pool Manager，避免循环依赖。
- BP_DamageTextComp 已经依赖 WBP（单向依赖），它监听该委托是合理扩展。

### 4. BP_TextComp 通过委托监听，不直接调用 WBP

在 `BP_DamageTextComp` 的 `SetDamageText` 事件图中：

1. `Get User Widget Object`
2. `Cast to WBP_DamageText`
3. `Call WBP_DamageText::PlayDamageText`
4. `Bind to OnDamageTextFinished`
   - 事件触发 → `ReturnToPool()`

### 5. WBP 增加 `PlayDamageText` 函数

`WBP_DamageText::PlayDamageText` 替代原来 `Construct` 中的动画播放逻辑：

```
Function: PlayDamageText
Inputs: Damage (double), Blocked (bool), Crit (bool)
Flow:
  ├── UpdateDamageText(Damage, Blocked, Crit)
  ├── PlayAnimation(DamageAnim)
  └── PlayAnimation(HitMessageAnim)
```

同时，`WBP_DamageText::Construct` 中不再播放动画，只做初始化（如设置默认可见性）。

---

## C++ Files to Modify

### 1. New: `DamageTextPoolManager.h` / `.cpp`

路径：

```
Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Pooling/DamageTextPoolManager.h
Plugins/RPGCore/Source/RPGCore/Private/RPGFramework/Pooling/DamageTextPoolManager.cpp
```

核心实现：

```cpp
void UDamageTextPoolManager::Initialize(ARPGPlayerController* Owner, TSubclassOf<UDamageTextComponent> Class)
{
    OwnerController = Owner;
    DamageTextClass = Class;
}

void UDamageTextPoolManager::PreWarm(int32 Count)
{
    if (!OwnerController || !DamageTextClass) return;

    for (int32 i = 0; i < Count; ++i)
    {
        UDamageTextComponent* Text = NewObject<UDamageTextComponent>(
            OwnerController, DamageTextClass);
        Text->SetActive(false);
        Text->SetHiddenInGame(true);
        Text->SetInUse(false);
        AvailablePool.Add(Text);
    }
}

UDamageTextComponent* UDamageTextPoolManager::Acquire(ACharacter* AttachTarget)
{
    if (!OwnerController || !DamageTextClass) return nullptr;

    for (auto& Text : AvailablePool)
    {
        if (Text && !Text->IsInUse())
        {
            AvailablePool.Remove(Text);
            ActivePool.Add(Text);
            Text->SetInUse(true);
            Text->SetActive(true);
            Text->SetHiddenInGame(false);
            Text->AttachToComponent(
                AttachTarget->GetRootComponent(),
                FAttachmentTransformRules::KeepRelativeTransform);
            return Text;
        }
    }

    // Pool miss: create new one
    if (ActivePool.Num() >= MaxPoolSize)
    {
        // Optional: discard the oldest active one to keep size
        return nullptr;
    }

    UDamageTextComponent* NewText = NewObject<UDamageTextComponent>(
        OwnerController, DamageTextClass);
    NewText->SetActive(true);
    NewText->SetHiddenInGame(false);
    NewText->SetInUse(true);
    NewText->AttachToComponent(
        AttachTarget->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    ActivePool.Add(NewText);

    return NewText;
}

void UDamageTextPoolManager::Release(UDamageTextComponent* Component)
{
    if (!Component) return;

    Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    Component->SetHiddenInGame(true);
    Component->SetActive(false);
    Component->SetInUse(false);

    ActivePool.Remove(Component);
    if (!AvailablePool.Contains(Component))
    {
        AvailablePool.Add(Component);
    }
}

void UDamageTextPoolManager::Shutdown()
{
    AvailablePool.Empty();
    ActivePool.Empty();
}
```

### 2. Modify: `DamageTextComponent.h` / `.cpp`

增加 `ReturnToPool()` 和 `IsInUse()` 标记。

```cpp
void UDamageTextComponent::ReturnToPool()
{
    if (UDamageTextPoolManager* Manager = Cast<UDamageTextPoolManager>(GetOuter()))
    {
        Manager->Release(this);
    }
}
```

> 注意：如果 `GetOuter()` 是 `ARPGPlayerController` 而不是 Manager，需要改为在 `Acquire` 时把 Manager 引用写入组件。

备选方案：在 `Acquire` 时调用 `Component->SetOuter(Manager)` 或通过 `SetOwner` 把 Manager 传给组件。更稳的方式是加一个 `SetPoolManager` 方法：

```cpp
UFUNCTION()
void SetPoolManager(UDamageTextPoolManager* Manager);

private:
    UPROPERTY()
    TObjectPtr<UDamageTextPoolManager> PoolManager;
```

### 3. Modify: `RPGPlayerController.h` / `.cpp`

增加：

```cpp
// RPGPlayerController.h
private:
    UPROPERTY()
    TObjectPtr<UDamageTextPoolManager> DamageTextPool;

public:
    UDamageTextPoolManager* GetDamageTextPool() const;

// ShowDamageNumber 内部改为从池取
```

```cpp
// RPGPlayerController.cpp
void ARPGPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (DamageTextCompClass)
    {
        DamageTextPool = NewObject<UDamageTextPoolManager>(this);
        DamageTextPool->Initialize(this, DamageTextCompClass);
        DamageTextPool->PreWarm(10);
    }

    // ... 原有逻辑 ...
}

void ARPGPlayerController::ShowDamageNumber_Implementation(
    ACharacter* TargetCharacter, float DamageAmount, bool bBlockedHit, bool bCriticalHit)
{
    if (!IsValid(TargetCharacter) || !DamageTextPool) return;

    UDamageTextComponent* DamageText = DamageTextPool->Acquire(TargetCharacter);
    if (!DamageText) return;

    DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
}
```

---

## Blueprint Files to Modify

### 1. `WBP_DamageText`

#### 新增变量/委托

- 自定义委托：`OnDamageTextFinished`（类型：自定义事件，无参数）

#### 新增函数：`PlayDamageText`

输入：

- `Damage` (double)
- `Blocked` (bool)
- `Crit` (bool)

函数体：

```
[Function Entry]
  │
  ▼
UpdateDamageText(Damage, Blocked, Crit)
  │
  ▼
PlayAnimation(DamageAnim)
  │
  ▼
PlayAnimation(HitMessageAnim)
```

#### 修改 `Construct`

原 `Construct` 中的 `PlayAnimation` 节点移除。只保留视觉默认设置（如透明度、位置等）。

#### 新增 `OnAnimationFinished` 事件

对 `DamageAnim` 增加事件监听：

```
Event OnAnimationFinished(DamageAnim)
  │
  ▼
OnDamageTextFinished.Broadcast()
```

### 2. `BP_DamageTextComp`

#### 修改 `SetDamageText` 事件图

原逻辑：

```
GetUserWidgetObject → Cast to WBP_DamageText → UpdateDamageText
```

新逻辑：

```
GetUserWidgetObject → Cast to WBP_DamageText
  │
  ├── Call PlayDamageText(Damage, Blocked, Crit)
  │
  └── Bind to OnDamageTextFinished
        │
        ▼
  [Event OnDamageTextFinished]
        │
        ▼
  ReturnToPool()
```

---

## Pool Lifecycle

### 状态转换

```
[Uninitialized]  ← PreWarm →  [Idle in AvailablePool]
                                    │
                                    │ Acquire
                                    ▼
                           [Active on TargetCharacter]
                                    │
                                    │ Animation Finished
                                    ▼
                           [Idle in AvailablePool]
                                    │
                                    │ PlayerController Destroy
                                    ▼
                              [GC Collected]
```

### 边界情况

| 情况 | 处理 |
|------|------|
| **池子空且未到上限** | `Acquire` 新建组件，加入 ActivePool |
| **池子空且到上限** | 返回 `nullptr`，不显示（或者销毁最老的 active） |
| **组件动画结束未归还** | 通过 `OnAnimationFinished` 事件保证 |
| **目标角色在动画期间被销毁** | `DetachFromComponent` 后组件仍由 Pool Manager 持有，安全 |
| **PlayerController 被销毁** | Pool Manager 被 GC，所有组件一起回收 |
| **Pool Manager 被销毁前还有 Active 组件** | `Shutdown` 清空数组，组件自然 GC |

---

## Verification

### 1. 功能验证

- 命中敌人 → 伤害数字正常显示。
- 连续命中 10 次 → 池子足够时不再有 `NewObject` 调用。
- 连续命中 20 次 → 池 miss 后新建组件，但总数不超过 `MaxPoolSize`。
- 动画结束后 → 组件从目标角色脱离，隐藏，回归池子。
- 再次命中 → 复用池子中组件。

### 2. 性能验证

使用已有的 `SCOPE_CYCLE_COUNTER` 标记：

```
stat RPGCore
stat Aura
```

观察 `ShowDamageNumber` 相关耗时变化。池化后预期：

- 单发命中耗时不变（第一次需要 Attach + PlayAnimation）。
- 连续命中平均耗时下降（无 `NewObject` + `RegisterComponent`）。
- 无 GC 尖刺。

### 3. 内存验证

- 用 `stat memory` 或 `obj list` 观察 `BP_DamageTextComp` 实例数量。
- 命中 100 次后，实例数量应稳定在 `MaxPoolSize`（默认 32）附近，不再无限增长。

---

## Implementation Order

建议按以下顺序实现：

1. **Phase 1**：C++ 层修改
   - 新建 `DamageTextPoolManager`
   - 修改 `DamageTextComponent` 基类（`ReturnToPool` / `IsInUse`）
   - 修改 `RPGPlayerController`（初始化 Pool、从池取）
   - 编译通过，测试 Acquire/Release 接口

2. **Phase 2**：WBP 层修改
   - 新增 `PlayDamageText` 函数
   - 移除 `Construct` 中的动画播放
   - 新增 `OnDamageTextFinished` 委托
   - 新增 `OnAnimationFinished` 事件广播
   - 编译保存

3. **Phase 3**：BP_DamageTextComp 修改
   - 改为调用 `PlayDamageText`
   - 绑定 `OnDamageTextFinished` → `ReturnToPool`
   - 编译保存

4. **Phase 4**：验证
   - PIE 命中测试
   - 连续命中测试
   - 观察池大小和 GC 表现

---

## Open Decisions

| 决策 | 默认推荐 | 备注 |
|------|---------|------|
| 初始池大小 | 10 | 根据 AOE 最大同时命中数调整 |
| 最大池大小 | 32 | 防止极端场景下无限增长 |
| 上限溢出策略 | 丢弃最老的 active 组件 | 也可选择静默不显示 |
| 是否池化 Widget 内部实例 | 否 | 只池化 `UWidgetComponent`，让 Widget 随组件生命周期重建 |
| 是否使用 `FUserWidgetPool` | 否 | 当前架构使用 `UWidgetComponent`，改造成纯 `UUserWidget` 成本更高 |

---

## Related Files

```
Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/UI/Widgets/DamageTextComponent.h
Plugins/RPGCore/Source/RPGCore/Private/RPGFramework/UI/Widgets/DamageTextComponent.cpp
Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Player/RPGPlayerController.h
Plugins/RPGCore/Source/RPGCore/Private/RPGFramework/Player/RPGPlayerController.cpp
Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Pooling/DamageTextPoolManager.h       [NEW]
Plugins/RPGCore/Source/RPGCore/Private/RPGFramework/Pooling/DamageTextPoolManager.cpp      [NEW]

/Game/Blueprints/UI/FloatingText/BP_DamageTextComp.BP_DamageTextComp                    [BP MODIFY]
/Game/Blueprints/UI/FloatingText/WBP_DamageText.WBP_DamageText                          [BP MODIFY]
```

---

## Issue Link

Closes [RPGCore #4](https://github.com/WitherLzero/UE5-RPG-Core/issues/4): `[Perf] DamageTextComponent is NewObject+RegisterComponent per hit`

---

## 方案 B：Actor 池化（推荐替代架构）

### 诊断回顾

当前方案在实施中发现 `NewObject<UDamageTextComponent>(PlayerController, ...)` 因 `AController::AController()` 调用 `SetHidden(true)`，导致 `UWidgetComponent::UpdateWidgetOnScreen()` 中 `!(GetOwner()->IsHidden())` 失败，伤害数字不渲染。已通过 `SetHidden(false)` 确认该根因。

该问题暴露了**以 UWidgetComponent 作为瞬态 UI 载体**的架构风险：

1. Owner/Outer 影响渲染可见性，存在隐蔽的 Actor 状态依赖。
2. 跨 Actor 的 Attach/Detach 生命周期复杂。
3. `RegisterComponent/UnregisterComponent` 复用路径在 UE 5.x 中存在 Slate 层重建问题。

**方案 B** 引入 `ADamageTextActor`，其 `CreateDefaultSubobject<UWidgetComponent>`，池化的是 **Actor** 而非裸 Component。关键诀窍是 Actor 始终保持可见，池化状态只通过 WidgetComponent 的 `SetHiddenInGame` 控制，不触碰 Actor 的 `SetActorHiddenInGame`。

---

### 1. 架构概览

```
旧架构:
  击中 → PoolManager → NewObject<DamageTextComp>(PC) → Register → Attach/Detach → SetDamageText
                                               ↑ Outer 是 PC → IsHidden true → 不渲染

方案 B:
  击中 → PoolManager → SpawnActor<DamageTextActor> → SetActorLocation(hitPos) → SetDamageText
                           ↑ Actor 是 Outer，WidgetComp 是 DefaultSubobject
                           ↑ 池化也只切 WidgetComp::SetHiddenInGame，Actor 永远不 hidden
```

**核心思路（非常关键，是整个方案的前提）：**

```cpp
// 错误的池化方式：Actor 隐藏 → 它的所有组件都隐藏 → WidgetComp 的 GetOwner()->IsHidden() = true → 不渲染
PooledActor->SetActorHiddenInGame(true);     // ❌ 不能这样

// 正确的池化方式：Actor 保持可见，只隐藏 WidgetComponent
WidgetComp->SetHiddenInGame(true);           // ✅ IsVisible() = false → UWidgetComponent 不渲染
WidgetComp->SetActive(false);                // ✅ 禁止 Tick

// Acquire 时：
WidgetComp->SetHiddenInGame(false);          // IsVisible() = true → 恢复渲染
WidgetComp->SetActive(true);
```

| 维度 | 旧架构 | 方案 B |
|------|--------|--------|
| 池化对象 | UWidgetComponent | AActor（含 WidgetComponent） |
| 组件创建 | `NewObject(PC)` → Outer 非 Actor | `CreateDefaultSubobject(this)` → Outer = Actor |
| 坐标方式 | Attach/Detach 舞 | `SetActorLocation(hitPos)` |
| 池隐藏 | SetHiddenInGame + 不 Register | Actor 不隐藏，仅 WidgetComp::SetHiddenInGame(true) |
| Owner.IsHidden | ❌ PlayerController 为 true | ✅ Actor 永远 false |

---

### 2. 新增 C++ 类

#### 2.1 `ADamageTextActor` — 池化的 Actor

加在 `RPGFramework/Pooling/` 下，与 PoolManager 同级。

**头文件：**

```cpp
// Plugins/RPGCore/Source/RPGCore/Public/RPGFramework/Pooling/DamageTextActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "DamageTextActor.generated.h"

class UWidgetComponent;
class UDamageTextPoolManager;

/**
 * 浮动伤害数字 Actor。
 * 作为"伤害数字池"中的可复用元素。
 * 内含一个 WidgetComponent 用于渲染伤害文字。
 *
 * 池化关键：Actor 不设置 SetActorHiddenInGame，只通过 WidgetComp 控制可见性。
 */
UCLASS(Blueprintable)
class RPGCORE_API ADamageTextActor : public AActor
{
    GENERATED_BODY()

public:
    ADamageTextActor();

    /** 显示伤害数字 */
    UFUNCTION(BlueprintCallable)
    void SetDamageText(float Damage, bool bBlockedHit, bool bCriticalHit);

    /** 从池中取出时调用 */
    void Acquired();

    /** 归还到池时调用 */
    void Released();

    /** 设置 PoolManager 引用（Release 时回池用） */
    void SetPoolManager(UDamageTextPoolManager* InManager) { PoolManager = InManager; }

    /** PoolManager 引用 */
    UDamageTextPoolManager* GetPoolManager() const { return PoolManager; }

    /** 获取 WidgetComponent */
    UWidgetComponent* GetWidgetComponent() const { return DamageTextComp; }

protected:
    /** WidgetComponent — 真正的伤害文字渲染器 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UWidgetComponent> DamageTextComp;

    /** 根组件 */
    UPROPERTY()
    TObjectPtr<USceneComponent> SceneRoot;

private:
    /** 所属 PoolManager（用于动画结束后调用 Release） */
    UPROPERTY()
    TObjectPtr<UDamageTextPoolManager> PoolManager;
};
```

**实现文件：**

```cpp
// Plugins/RPGCore/Source/RPGCore/Private/RPGFramework/Pooling/DamageTextActor.cpp

#include "DamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "DamageTextPoolManager.h"

ADamageTextActor::ADamageTextActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // —— Root —
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    // —— WidgetComponent（默认子对象，Owener = this） —
    DamageTextComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageText"));
    DamageTextComp->SetupAttachment(SceneRoot);
    DamageTextComp->SetWidgetSpace(EWidgetSpace::Screen);
    DamageTextComp->SetDrawSize(FVector2D(500.0f, 500.0f));
    DamageTextComp->SetPivot(FVector2D(0.5f, 0.5f));

    // 开启 Tick When Offscreen（Screen 空间需要）
    DamageTextComp->SetTickWhenOffscreen(true);
}

void ADamageTextActor::Acquired()
{
    // Actor 保持 Hidden = false
    // 只恢复 WidgetComponent 的可见和活动
    DamageTextComp->SetHiddenInGame(false);
    DamageTextComp->SetActive(true);
    DamageTextComp->SetVisibility(true);
}

void ADamageTextActor::Released()
{
    // 只禁止 WidgetComponent，不动 Actor
    DamageTextComp->SetHiddenInGame(true);
    DamageTextComp->SetActive(false);
    DamageTextComp->SetVisibility(false);
}

void ADamageTextActor::SetDamageText(float Damage, bool bBlockedHit, bool bCriticalHit)
{
    // 转发到 BP 端的实现（BP_DamageTextActor 会 override 这个函数）
    // 或者通过 FindFunction 调用 BP 端函数
    IInterface* Interface = Cast<IInterface>(this);
    if (UFunction* Func = FindFunction(TEXT("BP_SetDamageText")))
    {
        struct FParms
        {
            float Damage;
            bool bBlockedHit;
            bool bCriticalHit;
        };
        FParms Parms{ Damage, bBlockedHit, bCriticalHit };
        ProcessEvent(Func, &Parms);
    }
}
```

---

#### 2.2 `UDamageTextPoolManager` — 改为 Actor 池

只改类型和 PreWarm/Acquire/Release 逻辑，接口不变。

```cpp
// DamageTextPoolManager.h — 关键改动
UCLASS()
class RPGCORE_API UDamageTextPoolManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(APlayerController* Owner,
                    TSubclassOf<ADamageTextActor> ActorClass);

    void PreWarm(int32 Count);

    /** 从池取出一个 Actor，SetActorLocation 后返回 */
    ADamageTextActor* Acquire(const FVector& WorldLocation);

    /** 归还 Actor 到池 */
    void Release(ADamageTextActor* Actor);

    void Shutdown();

private:
    UPROPERTY()
    TObjectPtr<APlayerController> OwnerController;

    UPROPERTY()
    TSubclassOf<ADamageTextActor> DamageTextActorClass;

    UPROPERTY()
    TArray<TObjectPtr<ADamageTextActor>> AvailablePool;

    UPROPERTY()
    TArray<TObjectPtr<ADamageTextActor>> ActivePool;

    int32 MaxPoolSize = 32;
};
```

```cpp
// DamageTextPoolManager.cpp — 关键改动

void UDamageTextPoolManager::Initialize(APlayerController* Owner,
    TSubclassOf<ADamageTextActor> ActorClass)
{
    OwnerController = Owner;
    DamageTextActorClass = ActorClass;
}

void UDamageTextPoolManager::PreWarm(int32 Count)
{
    if (!OwnerController || !DamageTextActorClass) return;

    for (int32 i = 0; i < Count; ++i)
    {
        ADamageTextActor* Actor = GetWorld()->SpawnActor<ADamageTextActor>(
            DamageTextActorClass);
        Actor->Released();                     // 初始隐藏
        Actor->SetPoolManager(this);
        AvailablePool.Add(Actor);
    }
}

ADamageTextActor* UDamageTextPoolManager::Acquire(const FVector& WorldLocation)
{
    ADamageTextActor* Actor = nullptr;

    if (AvailablePool.Num() > 0)
    {
        Actor = AvailablePool[0];
        AvailablePool.RemoveAt(0);
    }
    else
    {
        // 池 miss — 创建新的
        if (ActivePool.Num() >= MaxPoolSize)
        {
            // 可选：回收最老的一个
            return nullptr;
        }

        Actor = GetWorld()->SpawnActor<ADamageTextActor>(DamageTextActorClass);
        Actor->Released();
        Actor->SetPoolManager(this);
    }

    // 设置位置
    Actor->SetActorLocation(WorldLocation);

    // 激活显示
    Actor->Acquired();

    ActivePool.Add(Actor);
    return Actor;
}

void UDamageTextPoolManager::Release(ADamageTextActor* Actor)
{
    if (!Actor) return;

    Actor->Released();

    ActivePool.Remove(Actor);
    if (!AvailablePool.Contains(Actor))
    {
        AvailablePool.Add(Actor);
    }
}

void UDamageTextPoolManager::Shutdown()
{
    for (auto& Actor : ActivePool)  { Actor->Destroy(); }
    for (auto& Actor : AvailablePool) { Actor->Destroy(); }
    ActivePool.Empty();
    AvailablePool.Empty();
}
```

---

#### 2.3 RPGPlayerController 修改

`ShowDamageNumber` 改为从 Actor 池取 + 计算位置：

```cpp
// RPGPlayerController.h
private:
    UPROPERTY()
    TObjectPtr<UDamageTextPoolManager> DamageTextPool;

// RPGPlayerController.cpp
void ARPGPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (DamageTextActorClass)
    {
        DamageTextPool = NewObject<UDamageTextPoolManager>(this);
        DamageTextPool->Initialize(this, DamageTextActorClass);
        DamageTextPool->PreWarm(10);
    }
}

void ARPGPlayerController::ShowDamageNumber_Implementation(
    ACharacter* TargetCharacter, float Damage,
    bool bBlockedHit, bool bCriticalHit)
{
    if (!IsValid(TargetCharacter) || !DamageTextPool) return;

    // 计算世界坐标：目标角色中段上方
    FVector WorldLocation = TargetCharacter->GetActorLocation()
                          + FVector(0, 0, 100);

    // 从池获取 Actor
    ADamageTextActor* TextActor = DamageTextPool->Acquire(WorldLocation);
    if (!TextActor) return;

    // 设置伤害数据（由 BP 端 PlayAnimation）
    TextActor->SetDamageText(Damage, bBlockedHit, bCriticalHit);
}
```

---

### 3. Blueprint 变更

#### 3.1 新建：`BP_DamageTextActor`

**父类**: `ADamageTextActor`（C++ 类）

**结构**：
```
BP_DamageTextActor [Actor]
  └─ Root [SceneComponent]
      └─ DamageText [WidgetComponent]
          ├─ WidgetClass = WBP_DamageText
          ├─ Space = Screen
          ├─ DrawSize = (500, 500)
          ├─ Pivot = (0.5, 0.5)
          └─ TickWhenOffscreen = true
```

**Event Graph** — 直接复用到 `BP_DamageTextComp` 的 SetDamageText 逻辑：

```
BP_SetDamageText (Custom Event)
  Input: Damage (float), BlockedHit (bool), CriticalHit (bool)

  GetWidgetComponent → GetUserWidgetObject
    → Cast to WBP_DamageText
    → PlayDamageText(Damage, Blocked, Crit)
    → Bind OnDamageTextFinished
      │
      ▼
  OnDamageTextFinished:
    Get PoolManager → Release(Self)
```

**与当前 BP_DamageTextComp 对比**：

| 项目 | 旧（BP_DamageTextComp） | 新（BP_DamageTextActor） |
|------|------------------------|-------------------------|
| 宿主 | UDamageTextComponent (WidgetComp) | ADamageTextActor |
| 获取 Widget | `GetUserWidgetObject()` | `GetOwner()->GetComponentByClass<UWidgetComp>()` → `GetUserWidgetObject()` |
| 获取池 Manager | `GetProperty(PoolManager)` | `Get PoolManager`（Actor 上有引用） |
| 动画逻辑 | 完全一样 | 完全一样 |
| 坐标 | Attach/Detach | 无需关心（PoolManager 做） |

#### 3.2 保留 `WBP_DamageText`（不变）

`WBP_DamageText` 纯 UMG，只关心文字/颜色/动画，不感知宿主是 Component 还是 Actor。不需要改。

#### 3.3 可选保留 `BP_DamageTextComp`（如已创建）

`BP_DamageTextActor` 的 WidgetComponent 直接设 `WidgetClass = WBP_DamageText`，不经过 BP_DamageTextComp。

---

### 4. 完整数据流

```
[击杀/命中事件]
    │
    ▼
RPGPlayerController::ShowDamageNumber (NetMulticast, Reliable)
    │
    ├── 计算被击点世界坐标 (TargetActorLocation + Z offset)
    │
    ▼
UDamageTextPoolManager::Acquire(WorldLocation)
    │
    ├── AvailablePool 非空 → Pop
    └── AvailablePool 空 → SpawnActor<ADamageTextActor>
    │
    ▼
Actor->SetActorLocation(WorldLocation)      ← 没有 Attach/Detach 舞
Actor->Acquired()
    ├── WidgetComp->SetHiddenInGame(false)
    ├── WidgetComp->SetActive(true)
    └── WidgetComp->SetVisibility(true)
    │
    ▼
Actor->SetDamageText(Damage, bBlocked, bCritical)
    │
    ▼
[BP_DamageTextActor Event Graph]
    │
    ├── Get WidgetComp → GetUserWidgetObject → Cast to WBP_DamageText
    ├── PlayDamageText → UpdateDamageText + PlayAnimation
    └── Bind OnDamageTextFinished
    │
    ▼
WBP_DamageText::OnAnimationFinished → OnDamageTextFinished.Broadcast()
    │
    ▼
BP_DamageTextActor::OnDamageTextFinished
    │
    └── PoolManager->Release(this)
          │
          ▼
Actor->Released()
    ├── WidgetComp->SetHiddenInGame(true)
    ├── WidgetComp->SetActive(false)
    └── WidgetComp->SetVisibility(false)
    │
    ▼
AvailablePool.Add(Actor)  ← 等待下次复用
```

---

### 5. 池生命周期

```
[Uninitialized]
    │ PreWarm / SpawnActor
    ▼
[Spawned + Hidden]
    Actor: Active, not hidden
    WidgetComp: HiddenInGame(true), Active(false)
    │
    │ Acquire → SetActorLocation + Acquired()
    ▼
[Active on World]
    Actor: Active, not hidden
    WidgetComp: HiddenInGame(false), Active(true), Visible(true)
    │
    │ AnimationFinished → PoolManager->Release()
    ▼
[Released → Hidden again]
    Actor: Active, not hidden
    WidgetComp: HiddenInGame(true), Active(false), Visible(false)
    │
    │ Shutdown → Destroy()
    ▼
[Destroyed]
```

**关键约束**：Actor 的 `IsHidden()` 在整个生命周期中**始终返回 false**。这是保证 `UWidgetComponent::UpdateWidgetOnScreen()` 中 `!(GetOwner()->IsHidden())` 通过的唯一条件。

| 状态 | Actor 位置 | WidgetComp 可见 | 渲染 | 池 |
|------|-----------|-----------------|------|-----|
| Prewarmed | Default (0,0,0) | Hidden | 不渲染 | AvailablePool |
| Active | 命中点 | Visible | 显示动画 | ActivePool |
| Released | 命中点（不变） | Hidden | 不渲染 | AvailablePool |

---

### 6. 边界情况

| 情况 | 处理 |
|------|------|
| **高攻速 20 次/秒** | 池满时返回 nullptr，不显示超出的数字 |
| **池 miss** | SpawnActor 新建，不超过 MaxPoolSize |
| **动画结束时目标已死亡** | Actor 在 ActivePool 中，Manager 存有引用 → 正常 Release |
| **PoolManager 被销毁** | Shutdown → 所有 Actor Destroy |
| **网络多人** | `ShowDamageNumber` 是 NetMulticast，每客户端各自 Spawn，各自池化。WorldLocation 由 Server 发送，投影由 WidgetComponent 在客户端自动处理 |

---

### 7. 现有文件处理

| 文件 | 状态 | 原因 |
|------|------|------|
| `UDamageTextComponent.h/.cpp` | **保留** | `BP_DamageTextActor` 的 WidgetComponent 的 WidgetClass 指向 `WBP_DamageText`，Component 类本身不需要特殊子类化。需要确认 `BP_DamageTextComp` 是否被其他系统引用。如果无引用，可以删除。 |
| `UDamageTextPoolManager.h/.cpp` | **修改** | 类型从 `UDamageTextComponent*` 改为 `ADamageTextActor*` |
| `BP_DamageTextComp` | 可选删除 | 如果 `SetDamageText` 逻辑已搬到 `BP_DamageTextActor` 中 |
| `WBP_DamageText` | **保留不改** | 纯 UMG，不感知宿主变化 |

### 8. 新增文件清单

| 文件 | 类型 | 说明 |
|------|------|------|
| `DamageTextActor.h/.cpp` | C++ (RPGCore) | 池化的 Actor，含 WidgetComponent |
| `BP_DamageTextActor` | Blueprint | 继承 C++ Actor，处理 BP_SetDamageText + ReturnToPool |

---

### 9. 方案对比总结

| 对比项 | 当前方案（Component Pooling） | 方案 B（Actor Pooling） |
|--------|------------------------------|-------------------------|
| **池化对象** | 裸 UWidgetComponent | AActor（WidgetComp 是子对象） |
| **创建方式** | `NewObject(PC)` | `SpawnActor<Actor>` |
| **Owner/Outer** | PlayerController → IsHidden = true | Actor → IsHidden = false |
| **坐标** | Attach + Detach 舞 | `SetActorLocation` |
| **隐藏** | Unregister + Detach + SetHiddenInGame | 仅 WidgetComp->SetHiddenInGame |
| **类数量** | 3 C++ + 2 BP | 2 C++（保留 PoolMgr + 新增 Actor）+ 1 BP |
| **新增代码行** | 已有的 | ~120 行（Actor 30 + PoolMgr 改 40 + PC 改 10 + BP） |
| **需要改现有的** | 已做完 | PoolMgr 改类型，PC 改 Acquire 参数，新建 BP |

---

### 10. 实施顺序

1. **新建** `DamageTextActor.h/.cpp`（C++，RPGCore）
2. **修改** `DamageTextPoolManager.h/.cpp`（TArray 改类型，Acquire 改参数，PreWarm/Release 改逻辑）
3. **修改** `RPGPlayerController.cpp`（Acquire 调用改为传 WorldLocation）
4. **编译** 确认 C++ 通过
5. **新建** `BP_DamageTextActor`，挂 WidgetComponent，设 WidgetClass = WBP_DamageText，复写 BP_SetDamageText
6. **测试** PIE 命中，确认数字正常显示、动画结束回池、连续命中复用
7. **可选** 清理 `BP_DamageTextComp`、`UDamageTextComponent`

---
