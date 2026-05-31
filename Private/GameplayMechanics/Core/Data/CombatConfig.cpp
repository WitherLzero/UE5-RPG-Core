// Copyright rynnli


#include "GameplayMechanics/Core/Data/CombatConfig.h"


FName UCombatConfig::GetSocketNameByTag(const FGameplayTag& Tag) const
{
	return TagToSocketMap.FindChecked(Tag);
}
