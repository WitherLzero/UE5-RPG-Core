// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CharacterDataInterface.generated.h"



// This class does not need to be modified.
UINTERFACE()
class RPGCORE_API UCharacterDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGCORE_API ICharacterDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual int32 GetCharacterLevel() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FGameplayTag GetCharacterClassTag();
};
