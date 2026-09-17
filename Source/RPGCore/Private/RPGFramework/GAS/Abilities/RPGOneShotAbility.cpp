// Copyright rynnli


#include "RPGFramework/GAS/Abilities/RPGOneShotAbility.h"

void URPGOneShotAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 双保险：经 GiveAbilityAndActivateOnce 授予的能力由引擎通过 RemoveAfterActivation 自动清理。
	// 此处兜底：若能力被错误授予（普通 GiveAbility / 装备流等无移除语义的路径），
	// 在权威端手动清除，防止 spec 残留。
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
		{
			if (!Spec->RemoveAfterActivation)
			{
				UE_LOG(LogTemp, Warning, TEXT("[OneShot] Ability '%s' was granted without RemoveAfterActivation. Use ASC::AddCharacterOneShotAbilities or ActivateOneShotAbility to grant one-shot abilities."), *GetName());
				ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
			}
		}
	}
}
