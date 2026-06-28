// Copyright rynnli


#include "GameplayMechanics/ARPG/Components/ActionComponent.h"

#include "MotionWarpingComponent.h"
#include "AbilitySystemComponent.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "Abilities/GameplayAbilityTargetTypes.h"


UActionComponent::UActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}


void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	MotionWarpingComp = Owner->FindComponentByClass<UMotionWarpingComponent>();
	
	if (!MotionWarpingComp)
	{
		MotionWarpingComp = NewObject<UMotionWarpingComponent>(Owner);
		MotionWarpingComp->RegisterComponent();
	}
}

void UActionComponent::UpdateFacingTarget(const FVector& TargetLoc)
{
	MotionWarpingComp->AddOrUpdateWarpTargetFromLocation(FacingTargetName, TargetLoc);
}

void UActionComponent::InitActionComponent(UAbilitySystemComponent* ASC)
{
	if (ASC)
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(FRPGGameplayTags::Get().Event_Action_UpdateWarpingTarget).AddUObject(this, &UActionComponent::OnUpdateWarpingTargetEvent);
	}
}

void UActionComponent::OnUpdateWarpingTargetEvent(const FGameplayEventData* Payload)
{
	if (!Payload) return;

	FVector ResolvedTargetLoc = FVector::ZeroVector;
	bool bFoundValidLocation = false;

	// 优先级1: Target 是 AActor
	if (const AActor* TargetActor = Cast<AActor>(Payload->Target))
	{
		ResolvedTargetLoc = TargetActor->GetActorLocation();
		bFoundValidLocation = true;
	}
	// 优先级2: TargetData (HitResult 或 EndPoint)
	else if (Payload->TargetData.Num() > 0)
	{
		if (const FGameplayAbilityTargetData* FirstData = Payload->TargetData.Get(0))
		{
			if (FirstData->HasHitResult() && FirstData->GetHitResult())
			{
				ResolvedTargetLoc = FirstData->GetHitResult()->Location;
				bFoundValidLocation = true;
			}
			else if (FirstData->HasEndPoint())
			{
				ResolvedTargetLoc = FirstData->GetEndPointTransform().GetLocation();
				bFoundValidLocation = true;
			}
		}
	}

	if (bFoundValidLocation)
	{
		UpdateFacingTarget(ResolvedTargetLoc);
	}
}



