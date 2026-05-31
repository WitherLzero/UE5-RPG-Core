// Copyright rynnli


#include "GameplayMechanics/Core/Components/CombatComponent.h"

#include "GameFramework/Character.h"
#include "GameplayMechanics/Core/Data/CombatConfig.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UCombatComponent::RegisterWeaponMesh(USceneComponent* InMesh)
{
	WeaponMesh = InMesh;
}




TArray<FTaggedMontage> UCombatComponent::GetAttackMontages_Implementation()
{
	return AttackMontages;
}


UAnimMontage* UCombatComponent::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}


FVector UCombatComponent::GetCombatSocketLocation(FGameplayTag MontageTag) const
{
	if (CombatConfig)
	{
		const FName SocketName = CombatConfig->GetSocketNameByTag(MontageTag);
		if (!SocketName.IsNone())
		{
			if (WeaponMesh && WeaponMesh->DoesSocketExist(SocketName))
			{
				return WeaponMesh->GetSocketLocation(SocketName);
			}

			if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
			{
				return Char->GetMesh()->GetSocketLocation(SocketName);
			}
		}
	}

	return GetOwner()->GetActorLocation();
}

UNiagaraSystem* UCombatComponent::GetHitReactEffect_Implementation()
{
	return HitReactEffect;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}



