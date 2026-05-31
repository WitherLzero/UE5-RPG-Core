// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGGameplayAbilityBase.h"
#include "RPGFramework/GAS/RPGAbilityTypes.h"
#include "DamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API UDamageGameplayAbility : public URPGGameplayAbilityBase
{
	GENERATED_BODY()
	
protected:
	
	UFUNCTION(BlueprintPure, Category = "Damage")
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr, bool bApplyDebuff = true, float RadialFalloff = 1.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Damage", meta = (AutoCreateRefTerm = "InDamageTypes"))
	FDamageEffectParams MakeDamageEffectParamsFromCustomValues(
		AActor* TargetActor,
		const TMap<FGameplayTag, FScalableFloat>& InDamageTypes,
		bool bApplyDebuff,
		float RadialFalloff,
		TSubclassOf<UGameplayEffect> InDebuffCarrier,
		float InDebuffChance,
		float InDebuffDamage,
		float InDebuffDuration,
		float InDebuffFrequency) const;
	
	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType);
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category= "Ability|Effect")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Ability|Damage")
	TMap<FGameplayTag,FScalableFloat> DamageTypes;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category= "Ability|Debuff")
	TSubclassOf<UGameplayEffect> DebuffCarrier;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Debuff")
	float DebuffChance = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Debuff")
	float DebuffDamage = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Debuff")
	float DebuffFrequency = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Debuff")
	float DebuffDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Radial")
	bool bIsRadialDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Radial")
	float RadialDamageInnerRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Ability|Radial")
	float RadialDamageOuterRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Radial")
	FVector RadialDamageOrigin = FVector::ZeroVector;
};
