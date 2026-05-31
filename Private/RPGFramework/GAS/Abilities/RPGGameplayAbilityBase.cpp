// Copyright rynnli


#include "RPGFramework/GAS/Abilities/RPGGameplayAbilityBase.h"

#include "RPGFramework/GAS/AttributeSets/VitalAttributeSet.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

FString URPGGameplayAbilityBase::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum ", Level);
}

FString URPGGameplayAbilityBase::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FString URPGGameplayAbilityBase::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked Until Level: %d</>"), Level);
}

float URPGGameplayAbilityBase::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == UVitalAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float URPGGameplayAbilityBase::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}

void URPGGameplayAbilityBase::SendWarpingTargetEvent(const FVector& TargetLocation)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	FHitResult HitResult;
	HitResult.Location = TargetLocation;
	
	const FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);

	FGameplayEventData Payload;
	Payload.TargetData = TargetData;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		AvatarActor,
		FRPGGameplayTags::Get().Event_Action_UpdateWarpingTarget,
		Payload);
}
