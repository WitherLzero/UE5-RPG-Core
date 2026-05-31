// Copyright rynnli


#include "GameplayMechanics/Core/Actor/RPGProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "RPGCore/RPGCore.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayMechanics/Core/RPGAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ARPGProjectile::ARPGProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	Sphere= CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	
	
}


void ARPGProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPGProjectile, HomingTargetActor);
	DOREPLIFETIME(ARPGProjectile, HomingTargetLocation);
	DOREPLIFETIME(ARPGProjectile, bIsHomingToLocation);
	DOREPLIFETIME(ARPGProjectile, bIsHomingEnabled);
	DOREPLIFETIME(ARPGProjectile, CachedAccelerationMin);
	DOREPLIFETIME(ARPGProjectile, CachedAccelerationMax);
}

void ARPGProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnSphereOverlap);
	LoopingAudioComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound,GetRootComponent());
}

void ARPGProjectile::Destroyed()
{
	if (!bHit && !HasAuthority()) HandleOnHit();

	Super::Destroyed();
}


void ARPGProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetInstigator()) return;
	
	if (!bHit) HandleOnHit();

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			FGameplayEffectSpecHandle DamageEffectSpecHandle = URPGAbilitySystemLibrary::MakeDamageEffectSpec(DamageEffectParams);
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
		
		Destroy();
	}
	else  bHit=true; // Determine whether the clients spawned the FXs before server destroyed
	
	
}



void ARPGProjectile::HandleOnHit()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,ImpactEffect,GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,GetActorLocation(),FRotator::ZeroRotator);
	if (LoopingAudioComponent) LoopingAudioComponent->Stop();
	bHit = true;
}

void ARPGProjectile::SetHomingTargetActor(AActor* TargetActor)
{
	HomingTargetActor = TargetActor;
	bIsHomingToLocation = false;
}

void ARPGProjectile::SetHomingTargetLocation(const FVector& TargetLocation)
{
	HomingTargetLocation = TargetLocation;
	bIsHomingToLocation = true;
}

void ARPGProjectile::SetHomingEnabled(bool bEnabled, float AccelerationMin, float AccelerationMax)
{
	if (!HasAuthority()) return;

	CachedAccelerationMin = AccelerationMin;
	CachedAccelerationMax = AccelerationMax;
	bIsHomingEnabled = bEnabled;
	
	if (bIsHomingEnabled)
	{
		EnableHoming(CachedAccelerationMin, CachedAccelerationMax);
	}
}

void ARPGProjectile::OnRep_IsHomingEnabled()
{
	if (bIsHomingEnabled)
	{
		EnableHoming(CachedAccelerationMin, CachedAccelerationMax);
	}
}

void ARPGProjectile::EnableHoming(float AccelerationMin, float AccelerationMax)
{
	if (!ProjectileMovement) return;

	if (IsValid(HomingTargetActor))
	{
		ProjectileMovement->HomingTargetComponent = HomingTargetActor->GetRootComponent();
		ProjectileMovement->bIsHomingProjectile = true;
	}
	else if (bIsHomingToLocation)
	{
		if (!HomingTargetSceneComponent)
		{
			HomingTargetSceneComponent = NewObject<USceneComponent>(this);
		}
		HomingTargetSceneComponent->SetWorldLocation(HomingTargetLocation);
		ProjectileMovement->HomingTargetComponent = HomingTargetSceneComponent;
		ProjectileMovement->bIsHomingProjectile = true;
	}
	else
	{
		ProjectileMovement->bIsHomingProjectile = false;
		return;
	}

	ProjectileMovement->HomingAccelerationMagnitude = FMath::RandRange(AccelerationMin, AccelerationMax);
	
	GetWorldTimerManager().SetTimer(HomingTrackerTimer, this, &ThisClass::OnHomingTrackerTick, 0.1f, true);
}

void ARPGProjectile::OnHomingTrackerTick()
{
	if (IsValid(HomingTargetActor))
	{
		HomingTargetLocation = HomingTargetActor->GetActorLocation();
	}
	else if (!bIsHomingToLocation)
	{
		if (!HomingTargetSceneComponent)
		{
			HomingTargetSceneComponent = NewObject<USceneComponent>(this);
		}
		HomingTargetSceneComponent->SetWorldLocation(HomingTargetLocation);
		ProjectileMovement->HomingTargetComponent = HomingTargetSceneComponent;
		
		bIsHomingToLocation = true; 
	}
	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), HomingTargetLocation);
	if (DistanceSquared < 2500.f) 
	{
		GetWorldTimerManager().ClearTimer(HomingTrackerTimer);
		
		if (!bHit) HandleOnHit();
		
		if (HasAuthority())
		{
			Destroy(); 
		}
	}
}