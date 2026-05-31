// Copyright rynnli


#include "GameplayMechanics/Core/Components/VitalityComponent.h"


#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"


// Sets default values for this component's properties
UVitalityComponent::UVitalityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UVitalityComponent::InitVitality(UAbilitySystemComponent* ASC)
{
	
	if (IsValid(ASC))
	{
		URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(ASC);
		
		RPGASC->OnOutOfHealth.AddUObject(this,&ThisClass::HandleOutOfHealth);
		
		
	}
}



void UVitalityComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void UVitalityComponent::HandleOutOfHealth(AActor* Instigator)
{
	Die();
	OnDeath.Broadcast(GetOwner(),Instigator);
}

void UVitalityComponent::Die()
{
	bIsDead = true;
	MulticastHandleDeath();
}



void UVitalityComponent::MulticastHandleDeath_Implementation()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation());
	}
	
	PerformRagdoll();
}

void UVitalityComponent::PerformRagdoll()
{
	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		if (UCapsuleComponent* Capsule = Owner->GetCapsuleComponent())
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (USkeletalMeshComponent* Mesh = Owner->GetMesh())
		{
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(true);
			Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		}
	}
}



