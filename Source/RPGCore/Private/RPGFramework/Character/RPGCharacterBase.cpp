// Copyright rynnli


#include "RPGFramework/Character/RPGCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "InputActionValue.h"
#include "RPGCore/RPGCore.h"
#include "Components/CapsuleComponent.h"

ARPGCharacterBase::ARPGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile,ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	//GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	
	
}

void ARPGCharacterBase::Move(const FVector2D& InputAxis)
{
}


void ARPGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}




void ARPGCharacterBase::InitAbilityActorInfo()
{
}

void ARPGCharacterBase::InitDefaultAttributes() const
{
}

void ARPGCharacterBase::AddCharacterAbilities()
{
}


void ARPGCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	if (!GetAbilitySystemComponent()) return;
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle =GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass,Level,ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}


bool ARPGCharacterBase::OnNativeInput_Implementation(FGameplayTag Tag, ERPGInputEvent EventType,
                                                      FInputActionValue Value)
{
	return false;
}






