// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "RPGPlayerState.generated.h"


class ULevelConfig;
class UVitalAttributeSet;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValues*/)
/**
 * 
 */
UCLASS()
class RPGCORE_API ARPGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ARPGPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelConfig> LevelConfig;
	
	FOnPlayerStatChanged OnLevelChanged;
	FOnPlayerStatChanged OnXPChanged;
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UVitalAttributeSet> VitalAS;
	
private:
	UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_Level)
	int32 Level = 1;
	
	UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_XP)
	int32 XP = 1;
	
	UFUNCTION()
	void OnRep_Level(const int32& OldLevel);
	
	UFUNCTION()
	void OnRep_XP(const int32& OldXP);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UVitalAttributeSet* GetVitalAttributeSet() const { return VitalAS; }	
	
	FORCEINLINE int32 GetPlayerLevel()const { return Level; }
	FORCEINLINE int32 GetXP()const { return XP; }
	
	void LevelUp();
	
	void AddToLevel(int32 InLevel);
	void AddToXP(int32 InXP);
	
	void SetLevel(int32 InLevel);
	void SetXP(int32 InXP);
};
