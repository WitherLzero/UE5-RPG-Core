// Copyright rynnli


#include "RPGFramework/AI/RPGAIController.h"

#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RPGFramework/Types/RPGGameplayTags.h"


// Sets default values
ARPGAIController::ARPGAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComp");
}

void ARPGAIController::InitAIwithASC(UAbilitySystemComponent* ASC)
{
	if (IsValid(ASC))
	{
		ASC->RegisterGameplayTagEvent(FRPGGameplayTags::Get().State_Stunned,EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,&ThisClass::OnStunTagChanged);		
		ASC->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Effects_HitReact,EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,&ThisClass::OnHitReactTagChanged);
		
	}
}

void ARPGAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ARPGAIController::OnHitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"),NewCount > 0);
	}
}

void ARPGAIController::OnStunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsBool(FName("IsStunned"),NewCount > 0);
	}
}


