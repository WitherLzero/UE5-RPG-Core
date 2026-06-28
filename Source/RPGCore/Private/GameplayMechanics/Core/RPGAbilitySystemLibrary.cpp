// Copyright rynnli


#include "GameplayMechanics/Core/RPGAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/OverlapResult.h"
#include "GameplayMechanics/Core/Actor/RPGProjectile.h"
#include "RPGFramework/GAS/RPGAbilityTypes.h"
#include "Kismet/GameplayStatics.h"

#include "GameplayMechanics/Core/Components/CombatComponent.h"
#include "GameplayMechanics/Core/Components/VitalityComponent.h"
#include "GameplayMechanics/Core/Interaction/CombatInterface.h"
#include "RPGFramework/System/RPGFrameworkSettings.h"
#include "RPGFramework/Types/RPGGameplayTags.h"


UAbilityInfo* URPGAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const URPGFrameworkSettings* Settings = GetDefault<URPGFrameworkSettings>();

	if (Settings && !Settings->GlobalAbilityInfo.IsNull())
	{
		return Settings->GlobalAbilityInfo.LoadSynchronous();
	}
	
	return nullptr;
}

void URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
                                                          TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
                                                          const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, 
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), 
			FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (UVitalityComponent* VitalityComp =  Overlap.GetActor()->FindComponentByClass<UVitalityComponent>())
			{
				if (!VitalityComp->IsDead())
				{
					OutOverlappingActors.AddUnique(Overlap.GetActor());
				}
			}
		}
	}
}

void URPGAbilitySystemLibrary::GetClosestTargetsMax(int32 MaxTargets, const FVector& Origin, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets)
{
	OutClosestTargets = Actors;
	if (OutClosestTargets.Num() <= MaxTargets)
	{
		return;
	}
	OutClosestTargets.Sort([&Origin](const AActor& A, const AActor& B)
	{
		const double DistSqA = (A.GetActorLocation() - Origin).SquaredLength();
		const double DistSqB = (B.GetActorLocation() - Origin).SquaredLength();

		return DistSqA < DistSqB;
	});

	OutClosestTargets.SetNum(MaxTargets);
}

void URPGAbilitySystemLibrary::GetClosestTargets(const FVector& Origin, const TArray<AActor*>& Actors, TArray<AActor*>& OutSortedTargets)
{
	OutSortedTargets = Actors;
	OutSortedTargets.Sort([&Origin](const AActor& A, const AActor& B)
	{
		const double DistSqA = (A.GetActorLocation() - Origin).SquaredLength();
		const double DistSqB = (B.GetActorLocation() - Origin).SquaredLength();
		return DistSqA < DistSqB;
	});
}

bool URPGAbilitySystemLibrary::TraceAttackTrajectory(AActor* Instigator, const FGameplayTag& StartSocketTag,
	const FVector& TargetLocation, float TraceRadius, ETraceTypeQuery TraceChannel,FHitResult& OutHitResult)
{
	if (!Instigator || !Instigator->Implements<UCombatInterface>())
	{
		return false;
	}
	
	const FVector StartLocation = GetCombatSocketLocation(Instigator, StartSocketTag);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Instigator); 
	
	return UKismetSystemLibrary::SphereTraceSingle(
		Instigator,             
		StartLocation, 
		TargetLocation, 
		TraceRadius, 
		TraceChannel, 
		false,                  
		ActorsToIgnore, 
		EDrawDebugTrace::None,  
		OutHitResult, 
		true                   
	);
}


ARPGProjectile* URPGAbilitySystemLibrary::SpawnProjectileTowardsTarget(const UObject* WorldContextObject,
                                                                       TSubclassOf<ARPGProjectile> ProjectileClass, const FVector& SpawnLocation, const FVector& TargetLocation,
                                                                       bool bOverridePitch, float PitchOverride, AActor* Instigator, const FDamageEffectParams& DamageEffectParams)
{
	if (!WorldContextObject || !ProjectileClass) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	
	if (!Instigator->HasAuthority()) return nullptr;
	
	FRotator Rotation = (TargetLocation - SpawnLocation).Rotation();
	if (bOverridePitch) Rotation.Pitch = PitchOverride;
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());
	
	ARPGProjectile* Projectile = World->SpawnActorDeferred<ARPGProjectile>(
	ProjectileClass,
	SpawnTransform,
	Instigator,Cast<APawn>(Instigator),
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	Projectile->DamageEffectParams = DamageEffectParams;
	
	Projectile->FinishSpawning(SpawnTransform);
	
	return Projectile;
}

ARPGProjectile* URPGAbilitySystemLibrary::SpawnProjectileInDirection(const UObject* WorldContextObject,
	TSubclassOf<ARPGProjectile> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation,
	bool bOverridePitch, float PitchOverride, AActor* Instigator, const FDamageEffectParams& DamageEffectParams)
{
	if (!WorldContextObject || !ProjectileClass) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	
	if (!Instigator->HasAuthority()) return nullptr;
	
	FRotator NewRotation = SpawnRotation;
	if (bOverridePitch) NewRotation.Pitch = PitchOverride;
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(NewRotation.Quaternion());
	
	ARPGProjectile* Projectile = World->SpawnActorDeferred<ARPGProjectile>(
	ProjectileClass,
	SpawnTransform,
	Instigator,Cast<APawn>(Instigator),
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	Projectile->DamageEffectParams = DamageEffectParams;
	
	Projectile->FinishSpawning(SpawnTransform);
	
	return Projectile;
}

TArray<FRotator> URPGAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis( -Spread/2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; ++i)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis( DeltaSpread*i, Axis);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> URPGAbilitySystemLibrary::EvenlySpacedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis( -Spread/2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; ++i)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis( DeltaSpread*i, Axis);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}

TArray<FVector> URPGAbilitySystemLibrary::GetGroundRadialPoints(const UObject* WorldContextObject, const FVector& CenterLocation,
	int32 NumPoints, float Radius, float YawOverride, bool bIncludeCenter)
{
	TArray<FVector> TheoreticalPoints;
	if (NumPoints <= 0) return TheoreticalPoints;

	int32 RemainingPoints = NumPoints;

	if (bIncludeCenter)
	{
		TheoreticalPoints.Add(CenterLocation);
		RemainingPoints--;
	}

	if (RemainingPoints > 0)
	{
		float DeltaAngle = 360.f / RemainingPoints; 
		
		for (int32 i = 0; i < RemainingPoints; ++i)
		{
			float CurrentAngle = YawOverride + (DeltaAngle * i);
			FVector Direction = FVector::ForwardVector.RotateAngleAxis(CurrentAngle, FVector::UpVector);
			TheoreticalPoints.Add(CenterLocation + Direction * Radius);
		}
	}

	TArray<FVector> GroundedPoints;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return TheoreticalPoints;

	FCollisionQueryParams QueryParams;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic); 
	
	for (const FVector& Pt : TheoreticalPoints)
	{
		FVector RayStart = Pt + FVector(0.f, 0.f, 500.f);
		FVector RayEnd = Pt - FVector(0.f, 0.f, 500.f);

		FHitResult HitResult;
		if (World->LineTraceSingleByObjectType(HitResult, RayStart, RayEnd, ObjectQueryParams, QueryParams))
		{
			GroundedPoints.Add(HitResult.ImpactPoint);
		}
		else
		{
			GroundedPoints.Add(Pt);
		}
	}

	return GroundedPoints;
}

FGameplayEffectSpecHandle URPGAbilitySystemLibrary::MakeDamageEffectSpec(const FDamageEffectParams& DamageEffectParams)
{
	
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDebuffCarrier(EffectContextHandle, DamageEffectParams.DebuffCarrierClass);

	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	for (auto& Pair : DamageEffectParams.DamageTypes)
	{
		float ScaledDamage = Pair.Value.GetValueAtLevel(DamageEffectParams.AbilityLevel);
		ScaledDamage *= DamageEffectParams.RadialFalloff;
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,Pair.Key,ScaledDamage);
	}
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	
	
	return SpecHandle;
}

float URPGAbilitySystemLibrary::GetDamageFalloff(const FVector& Origin, const AActor* TargetActor, float InnerRadius, float OuterRadius)
{
	if (!TargetActor) return 0.f;

	const float Distance = FVector::Dist(Origin, TargetActor->GetActorLocation());

	if (Distance <= InnerRadius)
	{
		return 1.f;
	}
	if (Distance >= OuterRadius)
	{
		return 0.f;
	}
	const float Range = OuterRadius - InnerRadius;
	return 1.f - (Distance - InnerRadius) / Range;
}

bool URPGAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsBlockedHit();
	}
	return false;
}

bool URPGAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsCriticalHit();
	}
	return false;
}

bool URPGAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsSuccessfulDebuff();
	}
	return false;
}

TSubclassOf<UGameplayEffect> URPGAbilitySystemLibrary::GetDebuffCarrier(
	const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetDebuffCarrier();
	}
	return nullptr;
}

float URPGAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

bool URPGAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsRadialDamage();
	}
	return false;
}

float URPGAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector URPGAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

FGameplayTag URPGAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (EffectContext->GetDamageType().IsValid())
		{
			return *EffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FGameplayTag URPGAbilitySystemLibrary::GetDebuffType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* EffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (EffectContext->GetDebuffType().IsValid())
		{
			return *EffectContext->GetDebuffType();
		}
	}
	return FGameplayTag();
}

void URPGAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void URPGAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void URPGAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void URPGAbilitySystemLibrary::SetDebuffCarrier(FGameplayEffectContextHandle& EffectContextHandle,
	TSubclassOf<UGameplayEffect> DebuffCarrierClass)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDebuffCarrier(DebuffCarrierClass);
	}
}

void URPGAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDebuffDamage( InDamage);
	}
}

void URPGAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle,
	float InDuration)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDebuffDuration(InDuration);
	}
}

void URPGAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	float InFrequency)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDebuffFrequency(InFrequency);
	}
}

void URPGAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetRadialDamageOrigin(InOrigin);
	}
}

void URPGAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		EffectContext->SetDamageType(DamageType);
	}
}

void URPGAbilitySystemLibrary::SetDebuffType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDebuffType)
{
	if (FRPGGameplayEffectContext* EffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DebuffType = MakeShared<FGameplayTag>(InDebuffType);
		EffectContext->SetDebuffType(DebuffType);
	}
}

void URPGAbilitySystemLibrary::ApplyLooseTagToActor(AActor* TargetActor, FGameplayTag TagToApply)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
	{
		ASC->AddLooseGameplayTag(TagToApply);
	}
}

void URPGAbilitySystemLibrary::RemoveLooseTagFromActor(AActor* TargetActor, FGameplayTag TagToRemove)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

FTaggedMontage URPGAbilitySystemLibrary::PickRandomTaggedMontage(const TArray<FTaggedMontage>& Montages)
{
	const int32 Num = Montages.Num();
	if (Num <= 0) return FTaggedMontage();
	const int32 Index = FMath::RandRange(0,Num - 1);
	return Montages[Index];
}



void URPGAbilitySystemLibrary::ApplyEffectToSelf(UAbilitySystemComponent* ASC, AActor* Avatar, TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	FGameplayEffectContextHandle ContextHandle =ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(Avatar);
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass,Level,ContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

USkeletalMeshComponent* URPGAbilitySystemLibrary::GetWeapon(const AActor* CombatActor)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetWeapon(CombatActor);
	}
	return nullptr;
}

FVector URPGAbilitySystemLibrary::GetCombatSocketLocation(const AActor* CombatActor, const FGameplayTag& MontageTag)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetCombatSocketLocation(CombatActor, MontageTag);
	}
	return FVector::ZeroVector;
}

TArray<FTaggedMontage> URPGAbilitySystemLibrary::GetAttackMontages(const AActor* CombatActor)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetAttackMontages(CombatActor);
	}
	return TArray<FTaggedMontage>();
}

UAnimMontage* URPGAbilitySystemLibrary::GetHitReactMontage(const AActor* CombatActor)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetHitReactMontage(CombatActor);
	}
	return nullptr;
}

UNiagaraSystem* URPGAbilitySystemLibrary::GetHitReactEffect(const AActor* CombatActor)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetHitReactEffect(CombatActor);
	}
	return nullptr;
}

AActor* URPGAbilitySystemLibrary::GetCombatTarget(const AActor* CombatActor)
{
	if (CombatActor && CombatActor->Implements<UCombatInterface>())
	{
		return ICombatInterface::Execute_GetCombatTarget(CombatActor);
	}
	return nullptr;
}
