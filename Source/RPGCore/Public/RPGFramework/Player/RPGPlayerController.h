// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

class UDamageTextComponent;
class ADamageTextActor;
class UDamageTextPoolManager;
class URPGAbilitySystemComponent;
enum class ERPGInputEvent : uint8;
class URPGInputConfig;
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

	// ---- Cursor configuration (applied once in BeginPlay) ----
	// Genre-neutral: defaults match Aura (top-down). Third-person hosts
	// override in BP class defaults or ctor. Dynamic show/hide during
	// gameplay is NOT handled here — mechanics that need the cursor call
	// the engine APIs directly (see docs/adr/0003).
	UPROPERTY(EditDefaultsOnly, Category="Cursor")
	bool bShowCursor = true;

	UPROPERTY(EditDefaultsOnly, Category="Cursor")
	TEnumAsByte<EMouseCursor::Type> MouseCursorType = EMouseCursor::Default;

	/** true → FInputModeGameAndUI (top-down default); false → FInputModeGameOnly. */
	UPROPERTY(EditDefaultsOnly, Category="Cursor|InputMode")
	bool bUseGameAndUIMode = true;

	UPROPERTY(EditDefaultsOnly, Category="Cursor|InputMode")
	EMouseLockMode MouseLockMode = EMouseLockMode::DoNotLock;

	UPROPERTY(EditDefaultsOnly, Category="Cursor|InputMode")
	bool bHideCursorDuringCapture = false;

	/** DamageTextActor class for Actor-pooling (Scheme B). */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADamageTextActor> DamageTextActorClass;

	UPROPERTY()
	TObjectPtr<UDamageTextPoolManager> DamageTextPool;
	
	UFUNCTION(BlueprintPure)
	UDamageTextPoolManager* GetDamageTextPool() const { return DamageTextPool; }
	
	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> AbilitySystemComponent;
};
