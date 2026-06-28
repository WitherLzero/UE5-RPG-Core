// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RPGFramework/GAS/Data/AbilityInfo.h"
#include "RPGFrameworkSettings.generated.h"


/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="RPG Framework"))
class RPGCORE_API URPGFrameworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "RPG Data Assets | Ability Info")
	TSoftObjectPtr<UAbilityInfo> GlobalAbilityInfo;
};
