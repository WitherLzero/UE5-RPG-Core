// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/GAS/Abilities/DamageGameplayAbility.h"
#include "RPGProjectileSpell.generated.h"

class ARPGProjectile;
/**
 * 
 */
UCLASS()
class RPGCORE_API URPGProjectileSpell : public UDamageGameplayAbility
{
	GENERATED_BODY()

	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<ARPGProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 NumProjectiles = 5;
};
