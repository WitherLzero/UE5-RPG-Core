// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "RPGFramework/Interaction/CharacterDataInterface.h"
#include "RPGFramework/Interaction/InputInteractable.h"
#include "RPGCharacterBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class RPGCORE_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICharacterDataInterface, public IInputInteractable
{
	GENERATED_BODY()

public:
	ARPGCharacterBase();

	virtual void Move(const FVector2D& InputAxis);

protected:
	virtual void BeginPlay() override;
	
	virtual void InitAbilityActorInfo();
	virtual void InitDefaultAttributes() const;
	virtual void AddCharacterAbilities();
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
	
	

	
	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category="Input | Native")
	bool OnNativeInput(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value);
	
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="Attributes")
	TArray<TSubclassOf<UGameplayEffect>> DefaultAttributes;	
	
	UPROPERTY(EditAnywhere,Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities;
	
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
};
