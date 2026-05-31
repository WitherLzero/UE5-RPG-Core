// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEffectAssetTagsGet, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnOutOfHealthSignature, AActor* /*Instigator*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityGiven, URPGAbilitySystemComponent*);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAbilityStatusChangedSignature, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPassiveAbilityDeactivated, const FGameplayTag& /*AbilityTag*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityLevelChanged);


DECLARE_DELEGATE_OneParam(FAbilitySpecAction, const FGameplayAbilitySpec&);
/**
 * 
 */
UCLASS()
class RPGCORE_API URPGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void OnAbilityActorInfoSet();
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
	void ApplyActionToAbilities(const FAbilitySpecAction& Action);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static bool AbilityHasInputTag(const FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag);
	static bool AbilityHasAnyInputTag(const FGameplayAbilitySpec& Spec);
	static void AssignInputToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag);
	
	bool AbilityInputIsEmpty(const FGameplayTag& InputTag);
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	FGameplayAbilitySpec* GetSpecWithInputTag(const FGameplayTag& InputTag);
	
	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec) const;
	
	void UpdateAbilityStatuses(int32 Level);
	void UnlockOrUpgradeAbility(const FGameplayTag& AbilityTag);
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);
	
	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag);
	
	FOnEffectAssetTagsGet OnEffectAssetTagsGet;
	FOnOutOfHealthSignature OnOutOfHealth;
	FOnAbilityGiven OnAbilityGiven;
	FOnAbilityStatusChangedSignature OnAbilityStatusChanged;
	FOnAbilityEquipped OnAbilityEquipped;
	FOnPassiveAbilityDeactivated OnPassiveAbilityDeactivated;
	
	UPROPERTY(BlueprintAssignable)
	FOnAbilityLevelChanged OnAbilityLevelChanged;
	
	bool bStartupAbilitiesGiven = false;
	
protected:
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
	
	UFUNCTION(Client, Reliable)
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& NewInputTag, const FGameplayTag& PreviousInputTag);	
	
	virtual void OnRep_ActivateAbilities() override;
	
private:
	static void ClearAbilityInput(FGameplayAbilitySpec* Spec);
};
