// Copyright rynnli


#include "RPGFramework/GAS/Abilities/DamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


FDamageEffectParams UDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor, bool bApplyDebuff , float RadialFalloff) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.DamageTypes = DamageTypes;
	Params.AbilityLevel = GetAbilityLevel();
	if (bApplyDebuff)
	{
		Params.DebuffCarrierClass = DebuffCarrier;
		Params.DebuffChance = DebuffChance;
		Params.DebuffDamage = DebuffDamage;
		Params.DebuffDuration = DebuffDuration;
		Params.DebuffFrequency = DebuffFrequency;
	}
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialFalloff = RadialFalloff;
		Params.RadialDamageOrigin = RadialDamageOrigin;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
}

FDamageEffectParams UDamageGameplayAbility::MakeDamageEffectParamsFromCustomValues(
	AActor* TargetActor,
	const TMap<FGameplayTag, FScalableFloat>& InDamageTypes,
	bool bApplyDebuff,
	float RadialFalloff,
	TSubclassOf<UGameplayEffect> InDebuffCarrier,
	float InDebuffChance,
	float InDebuffDamage,
	float InDebuffDuration,
	float InDebuffFrequency) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.DamageTypes = InDamageTypes;
	Params.AbilityLevel = GetAbilityLevel();
	if (bApplyDebuff)
	{
		Params.DebuffCarrierClass = InDebuffCarrier;
		Params.DebuffChance = InDebuffChance;
		Params.DebuffDamage = InDebuffDamage;
		Params.DebuffDuration = InDebuffDuration;
		Params.DebuffFrequency = InDebuffFrequency;
	}
	if (bIsRadialDamage)
	{
		Params.RadialFalloff = RadialFalloff;
	}
	return Params;
}

float UDamageGameplayAbility::GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType)
{
	checkf(DamageTypes.Contains(DamageType), TEXT("GameplayAbilit [%s] does not contain DamageType [%s]"), *GetNameSafe(this), *DamageType.ToString());
	return DamageTypes[DamageType].GetValueAtLevel(InLevel);
}
