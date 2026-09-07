// Copyright rynnli

#include "GameplayMechanics/Core/Components/CursorHighlightComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "RPGFramework/Interaction/EnemyInterface.h"
#include "RPGFramework/Stats/RPGCoreStats.h"
#include "RPGFramework/Types/RPGGameplayTags.h"

// File-local stat; the header-wide STAT_CursorTrace was retired when the
// trace moved out of ARPGPlayerController (see docs/adr/0003).
DECLARE_CYCLE_STAT(TEXT("CursorHighlight"), STAT_CursorHighlight, STATGROUP_RPGCore);

UCursorHighlightComponent::UCursorHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCursorHighlightComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only locally-controlled clients own a mouse cursor.
	CachedPC = ResolveController();
	SetComponentTickEnabled(CachedPC.IsValid());
}

void UCursorHighlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SCOPE_CYCLE_COUNTER(STAT_CursorHighlight);

	if (!CachedPC.IsValid())
	{
		CachedPC = ResolveController();
		if (!CachedPC.IsValid())
		{
			return;
		}
	}

	CursorTrace();
}

void UCursorHighlightComponent::CursorTrace()
{
	if (IsTraceBlocked())
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->UnHighlightActor();
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}

	// Rate-limit the scene query (default 30 Hz).
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastTraceTime) < (1.f / TraceRate))
	{
		return;
	}
	LastTraceTime = CurrentTime;

	FHitResult CursorHit;
	CachedPC->GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
}

bool UCursorHighlightComponent::IsTraceBlocked() const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	return ASC && ASC->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_CursorTrace);
}

APlayerController* UCursorHighlightComponent::ResolveController() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn ? OwnerPawn->GetController<APlayerController>() : nullptr;
}
