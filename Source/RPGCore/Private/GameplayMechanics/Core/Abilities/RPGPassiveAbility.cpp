// Copyright rynnli


#include "GameplayMechanics/Core/Abilities/RPGPassiveAbility.h"

#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"

void URPGPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(ActorInfo->AbilitySystemComponent))
	{
		ASC->OnPassiveAbilityDeactivated.AddUObject(this, &ThisClass::ReceiveDeactivate);
	}
}

void URPGPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
