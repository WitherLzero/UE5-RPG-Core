// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelConfig.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API ULevelConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	virtual int32 FindLevelForXP(int32 XP) const ;
	virtual int32 GetXPRequirement(int32 Level) const;
	
	virtual int32 GetMaxLevel()const;
};
