// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "UserWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API UUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerSet();
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
public:
	UFUNCTION(blueprintCallable)
	virtual void SetWidgetController(UObject* InWidgetController);
};
