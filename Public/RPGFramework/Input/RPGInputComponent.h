// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "RPGInputConfig.h"
#include "RPGInputComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGCORE_API URPGInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URPGInputComponent();
	
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindTaggedAction(const URPGInputConfig* InputConfig,UserClass* Object,
		PressedFuncType PressedFunc,ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void URPGInputComponent::BindTaggedAction(const URPGInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);
	
	for (const FRPGInputAction& Action : InputConfig->TaggedInputActions)
	{
		if (PressedFunc)
		{
			BindAction(Action.InputAction,ETriggerEvent::Started,Object,PressedFunc,Action.InputTag);
		}
		if (ReleasedFunc)
		{
			BindAction(Action.InputAction,ETriggerEvent::Completed,Object,ReleasedFunc,Action.InputTag);
		}
		if (HeldFunc)
		{
			BindAction(Action.InputAction,ETriggerEvent::Triggered,Object,HeldFunc,Action.InputTag);
		}
		
	}
}
