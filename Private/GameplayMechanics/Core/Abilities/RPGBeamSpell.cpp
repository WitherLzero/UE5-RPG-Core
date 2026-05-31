// Copyright rynnli


#include "GameplayMechanics/Core/Abilities/RPGBeamSpell.h"

#include "GameplayMechanics/Core/RPGAbilitySystemLibrary.h"

void URPGBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void URPGBeamSpell::StoreAdditionalTargets(AActor* OriginActor, TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(OriginActor);
	
	TArray<AActor*> OverlappingActors;
	URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		OriginActor->GetActorLocation());
	
	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	
	URPGAbilitySystemLibrary::GetClosestTargetsMax(NumAdditionalTargets,OriginActor->GetActorLocation(),OverlappingActors, OutAdditionalTargets);
}
