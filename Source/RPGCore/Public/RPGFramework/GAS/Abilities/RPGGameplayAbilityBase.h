// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RPGGameplayAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API URPGGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category= "Input")
	FGameplayTag StartupInputTag;
	
	virtual FText GetDescription(int32 Level);
	virtual FText GetNextLevelDescription(int32 Level);
	static FText GetLockedDescription(int32 Level);
	
protected:
	float GetManaCost(float InLevel = 1.f) const;
	float GetCooldown(float InLevel = 1.f) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void SendWarpingTargetEvent(const FVector& TargetLocation);
};
