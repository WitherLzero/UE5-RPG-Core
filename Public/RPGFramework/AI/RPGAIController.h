// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "RPGAIController.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;

UCLASS()
class RPGCORE_API ARPGAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARPGAIController();

	void InitAIwithASC(UAbilitySystemComponent* ASC);
	
	

protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComp;

	virtual void OnPossess(APawn* InPawn) override;
	
	void OnHitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void OnStunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

};
