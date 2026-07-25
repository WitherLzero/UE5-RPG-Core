// Copyright rynnli

#include "RPGFramework/GAS/TargetActor/RPGTargetActor_Indicator.h"
#include "RPGFramework/Stats/RPGCoreStats.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "GameFramework/PlayerController.h"

ARPGTargetActor_Indicator::ARPGTargetActor_Indicator()
{
	// Tick every frame to follow the cursor.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	
	DecalComp->DecalSize = FVector(10.f, 200.f, 200.f); 
	DecalComp->SetVisibility(false); // Hidden until StartTargeting reveals it.

	// Only the locally-controlled client owns the mouse cursor.
	ShouldProduceTargetDataOnServer = false;
}

void ARPGTargetActor_Indicator::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	
	// Show the decal ring once targeting begins.
	DecalComp->SetVisibility(true);
}

void ARPGTargetActor_Indicator::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_TargetIndicatorTick);
	Super::Tick(DeltaTime);

	// Poll cursor location every frame to reposition the decal.
	// The result is cached so ConfirmTargetingAndContinue can reuse it.
	if (APlayerController* PC =  OwningAbility->GetActorInfo().PlayerController.Get())
	{
		PC->GetHitResultUnderCursor(ECC_Visibility, false, CachedHitResult);

		if (CachedHitResult.bBlockingHit)
		{
			SetActorLocation(CachedHitResult.Location);
		}
	}
}

void ARPGTargetActor_Indicator::ConfirmTargetingAndContinue()
{
	if (IsConfirmTargetingAllowed() && ShouldProduceTargetData())
	{
		// Use the cached trace from Tick if available; fall back to a fresh query otherwise.
		if (!CachedHitResult.bBlockingHit)
		{
			if (APlayerController* PC = OwningAbility->GetActorInfo().PlayerController.Get())
			{
				PC->GetHitResultUnderCursor(ECC_Visibility, false, CachedHitResult);
			}
		}

		if (!CachedHitResult.bBlockingHit) return;

		// Build the GAS target-data handle from the cursor location.
		FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
		LocationData->TargetLocation.LiteralTransform = FTransform(CachedHitResult.Location);
		LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		
		FGameplayAbilityTargetDataHandle DataHandle;
		DataHandle.Add(LocationData);

		// Broadcast locally; WaitTargetData receives this and resumes the ability.
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
	
}
