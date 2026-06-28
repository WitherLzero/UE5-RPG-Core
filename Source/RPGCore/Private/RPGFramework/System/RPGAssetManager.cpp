// Copyright rynnli


#include "RPGFramework/System/RPGAssetManager.h"

#include "AbilitySystemGlobals.h"

#include "RPGFramework/Types/RPGGameplayTags.h"

URPGAssetManager& URPGAssetManager::Get()
{
	check(GEngine);
	return  *Cast<URPGAssetManager>(GEngine->AssetManager);
}

void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	FRPGGameplayTags::InitializeNativeGameplayTags();
	
	UAbilitySystemGlobals::Get().InitGlobalData();
}
