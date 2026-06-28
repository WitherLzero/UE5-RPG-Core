// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RPGInputConfig.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERPGInputEvent: uint8
{
	IE_Pressed,
	IE_Released,
	IE_Held
};

USTRUCT(BlueprintType)
struct FRPGInputAction
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

UCLASS()
class RPGCORE_API URPGInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TArray<FRPGInputAction> TaggedInputActions;
};
