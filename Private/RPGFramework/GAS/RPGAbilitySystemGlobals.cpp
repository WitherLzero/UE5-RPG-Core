// Copyright rynnli


#include "RPGFramework/GAS/RPGAbilitySystemGlobals.h"

#include "RPGFramework/GAS/RPGAbilityTypes.h"

FGameplayEffectContext* URPGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FRPGGameplayEffectContext();
}
