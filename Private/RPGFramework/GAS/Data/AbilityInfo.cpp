// Copyright rynnli


#include "RPGFramework/GAS/Data/AbilityInfo.h"


FRPGAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FRPGAbilityInfo& Info : AbilityInfos)
	{
		if (Info.AbilityTag.MatchesTagExact(AbilityTag))
		{
			return Info;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find Info for AbilityTag [%s] on AbilityInfo [%s]."), *AbilityTag.ToString(),*GetNameSafe(this));
	}
	
	return FRPGAbilityInfo();
}
