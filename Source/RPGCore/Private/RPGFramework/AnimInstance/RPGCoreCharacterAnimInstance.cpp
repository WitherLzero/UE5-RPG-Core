// Copyright rynnli

#include "RPGFramework/AnimInstance/RPGCoreCharacterAnimInstance.h"

#include "RPGFramework/Character/RPGCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void URPGCoreCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ARPGCharacterBase>(GetOwningActor());
	OwningMovementComponent = OwningCharacter ? OwningCharacter->GetCharacterMovement() : nullptr;
}

void URPGCoreCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;
}

float URPGCoreCharacterAnimInstance::GetGroundSpeed() const
{
	return GroundSpeed;
}

bool URPGCoreCharacterAnimInstance::GetHasAcceleration() const
{
	return bHasAcceleration;
}