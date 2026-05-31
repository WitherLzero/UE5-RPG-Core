// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CombatConfig.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API UCombatConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TMap<FGameplayTag,FName> TagToSocketMap;
	
	UFUNCTION(BlueprintCallable)
	FName GetSocketNameByTag(const FGameplayTag& Tag) const;
};
