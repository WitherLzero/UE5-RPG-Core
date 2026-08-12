// Copyright rynnli

#include "RPGFramework/AnimInstance/RPGCoreLinkedAnimLayer.h"

#include "RPGFramework/AnimInstance/RPGCoreCharacterAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

URPGCoreCharacterAnimInstance* URPGCoreLinkedAnimLayer::GetCharacterAnimInstance() const
{
	if (const USkeletalMeshComponent* OwningComponent = GetOwningComponent())
	{
		return Cast<URPGCoreCharacterAnimInstance>(OwningComponent->GetAnimInstance());
	}
	return nullptr;
}