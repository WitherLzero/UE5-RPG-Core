// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputInteractable.generated.h"

enum class ERPGInputEvent: uint8;
struct FInputActionValue;
struct FGameplayTag;

// This class does not need to be modified.
UINTERFACE()
class UInputInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGCORE_API IInputInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool HandleNativeInput(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value);
};
