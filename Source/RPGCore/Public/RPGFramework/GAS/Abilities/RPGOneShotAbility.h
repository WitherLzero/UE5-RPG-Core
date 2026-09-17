// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/GAS/Abilities/RPGGameplayAbilityBase.h"
#include "RPGOneShotAbility.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API URPGOneShotAbility : public URPGGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
