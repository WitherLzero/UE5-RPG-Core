// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

class UDamageTextComponent;
class URPGAbilitySystemComponent;
enum class ERPGInputEvent : uint8;
class URPGInputConfig;
class IEnemyInterface;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class RPGCORE_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ARPGPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client,Reliable)
	void ShowDamageNumber(ACharacter* TargetCharacter, float DamageAmount, bool bBlockedHit, bool bCriticalHit);
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
private:
	// helpers
	void UpdateMouse();
	void CursorTrace();
	 
	// IA Callbacks
	void OnInputTagPressed(FGameplayTag InputTag);
	void OnInputTagReleased(FGameplayTag InputTag);
	void OnInputTagHeld(const FInputActionValue& InputActionValue,FGameplayTag InputTag);
	
	// Input handler 
	void ProcessInputTag(FGameplayTag InputTag, ERPGInputEvent EventType, const FInputActionValue& InputActionValue = FInputActionValue());
	
	URPGAbilitySystemComponent* GetASC();
	
	UPROPERTY(EditAnywhere, Category= "Input")
	TObjectPtr<UInputMappingContext> CurrentMappingContext;
	
	UPROPERTY(EditDefaultsOnly,Category= "Input")
	TObjectPtr<URPGInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextCompClass;
	
	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> AbilitySystemComponent;
	
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;
	
public:
	bool GetCursorHit(FHitResult& HitResult);
	bool HitEnemyActor() const { return ThisActor?true:false;}
};
