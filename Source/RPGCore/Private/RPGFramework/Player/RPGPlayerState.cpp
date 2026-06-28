// Copyright rynnli


#include "RPGFramework/Player/RPGPlayerState.h"
#include "Net/UnrealNetwork.h"

#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/GAS/AttributeSets/VitalAttributeSet.h"

ARPGPlayerState::ARPGPlayerState()
{
	NetUpdateFrequency = 100.f;
	
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	VitalAS = CreateDefaultSubobject<UVitalAttributeSet>("VitalAttributeSet");

}

void ARPGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ARPGPlayerState,Level);
	DOREPLIFETIME(ARPGPlayerState,XP);
}

void ARPGPlayerState::OnRep_Level(const int32& OldLevel)
{
	OnLevelChanged.Broadcast(Level);
}

void ARPGPlayerState::OnRep_XP(const int32& OldXP)
{
	OnXPChanged.Broadcast(XP);
}

void ARPGPlayerState::LevelUp()
{
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->UpdateAbilityStatuses(Level);
	if (VitalAS)
	{
		VitalAS->bTopOfHealth = true;
		VitalAS->bTopOfMana = true;
	}
}

void ARPGPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChanged.Broadcast(Level);
}

void ARPGPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChanged.Broadcast(XP);
}

void ARPGPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChanged.Broadcast(Level);
}

void ARPGPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChanged.Broadcast(XP);
}
