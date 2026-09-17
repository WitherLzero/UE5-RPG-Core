# PRD: 武器 Actor 基类（ARPGWeaponBase）

## 背景

课程参考仓库 `vinceright3/WarriorRPG` commit `cff1059` 创建 `AWarriorWeaponBase`（Actor 武器基类）。设计决策已定案于 `docs/adr/0005-weapon-actor-base.md`，本 PRD 为其实施规格。本次**只沉 shell**：命中窗口语义、生成/挂载逻辑、战斗契约（Q3）均属后续课程 commit 范畴。

## 方案

`ARPGWeaponBase : AActor` 下沉至 RPGCore `GameplayMechanics/Core/Weapon/`，静态网格作根 + 碰撞盒，与 CombatComponent / VitalityComponent 同层。对 Aura 纯 additive。

## User Stories

1. 作为 **宿主项目开发者**，我想让武器蓝图（如 `BP_HeroAxe`）直接继承 Core 武器基类，获得网格 + 碰撞盒的标准结构，无需自建 C++ 类。
2. 作为 **近战玩法开发者**，我想拿到武器碰撞盒引用，以便后续课程接入命中窗口时开/关 Overlap。
3. 作为 **框架维护者**，我希望本类零业务假设（无 GAS 耦合、无生成逻辑），保持纯机制层。

## Implementation Decisions

### 新文件（2 个）

`Source/RPGCore/Public/GameplayMechanics/Core/Weapon/RPGWeaponBase.h` 与对应 `.cpp`：

```cpp
// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGWeaponBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class RPGCORE_API ARPGWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ARPGWeaponBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<UBoxComponent> WeaponCollisionBox;

public:
	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const { return WeaponCollisionBox; }
};
```

- 构造函数：`PrimaryActorTick.bCanEverTick = false`；`WeaponMesh` 为根（`SetRootComponent`）；`WeaponCollisionBox` 挂载根上、`SetBoxExtent(FVector(20.f))`、`SetCollisionEnabled(ECollisionEnabled::NoCollision)`（默认关闭，命中窗口语义后续 commit 引入）。
- 风格对齐 RPGCore 现有文件：`TObjectPtr`、`Tab` 缩进；`.cpp` include 顺序与同类一致。

### 明确不做（本次边界）

- ❌ 命中窗口开/关语义方法（等课程 AnimNotify 相关 commit）；
- ❌ Spawn/Attach 辅助逻辑（生成由 `GA_SpawnWeapon` BP 图承担，属后续映射）；
- ❌ 修改 `ICombatInterface` / `UCombatComponent`（Q3 已挂号于 ADR-0005"待后续触发"）；
- ❌ Warrior 项目层 C++ 子类（BP 直接继承 Core 基类）。

## 验收标准

- [ ] WarriorEditor Win64 Development 编译通过（exit 0），新代码零 warning；
- [ ] 仅新增上述 2 个文件，无任何既有文件改动；
- [ ] Aura 编译不受影响（纯新增，无接口变更）。

## 相关文档

- `docs/adr/0005-weapon-actor-base.md`（设计决策 + Q3 后续触发记录）
- `docs/adr/0004-one-shot-abilities.md`（武器生成链路：One-Shot 授予）
