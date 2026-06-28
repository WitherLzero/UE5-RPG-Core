// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RPGAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API URPGAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static URPGAssetManager& Get();
	
protected:
	virtual void StartInitialLoading() override;
};
