// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameplayMechanics/Core/Components/CombatComponent.h"
#include "CombatInterface.generated.h"

class UNiagaraSystem;
class UAnimMontage;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 纯粹的战斗数据契约层�?
 * 宿主 Actor 负责实现这些接口，并在内部“路由（转发）”给自身特定的组件�?
 * 保证了技能等外界调用者与宿主内部结构的绝对解耦�?
 */

class RPGCORE_API ICombatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	USkeletalMeshComponent* GetWeapon() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	TArray<FTaggedMontage> GetAttackMontages() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	UAnimMontage* GetHitReactMontage() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	UNiagaraSystem* GetHitReactEffect() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Combat Interface")
	AActor* GetCombatTarget() const;

	/** 设置当前战斗目标（基于妥协的推入状态方法，建议未来�?GameplayEvent 下发替代�?*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	void SetCombatTarget(AActor* InTarget);
};
