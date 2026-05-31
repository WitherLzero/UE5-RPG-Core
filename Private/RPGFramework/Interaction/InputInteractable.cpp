// Copyright rynnli


#include "RPGFramework/Interaction/InputInteractable.h"

#include "GameplayTagContainer.h"
#include "InputActionValue.h"


// Add default functionality here for any IInputInteractable functions that are not pure virtual.
bool IInputInteractable::HandleNativeInput(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value)
{
	return false;
}
