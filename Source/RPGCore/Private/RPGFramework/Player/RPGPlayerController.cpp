// Copyright rynnli


#include "RPGFramework/Player/RPGPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGFramework/Character/RPGCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/Input/RPGInputComponent.h"
#include "RPGFramework/Pooling/DamageText/DamageTextPoolManager.h"
#include "RPGFramework/Stats/RPGCoreStats.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "RPGFramework/Pooling/DamageText/DamageTextActor.h"
#include "Containers/Ticker.h"


ARPGPlayerController::ARPGPlayerController()
{
	bReplicates = true;
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_PlayerTick);
	Super::PlayerTick(DeltaTime);
	
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	check(CurrentMappingContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext( CurrentMappingContext,0);
	}
	
	// Initialize damage text pool (Actor-pooling: Scheme B)
	if (DamageTextActorClass)
	{
		DamageTextPool = NewObject<UDamageTextPoolManager>(this);
		DamageTextPool->Initialize(this, DamageTextActorClass);
		DamageTextPool->PreWarm(10);
	}
	
	// Apply cursor configuration once (genre-neutral, see docs/adr/0003).
	// Dynamic show/hide during gameplay is owned by the mechanics that need
	// the cursor — they call the engine APIs directly.
	bShowMouseCursor = bShowCursor;
	DefaultMouseCursor = MouseCursorType;
	if (bUseGameAndUIMode)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(MouseLockMode);
		InputModeData.SetHideCursorDuringCapture(bHideCursorDuringCapture);
		SetInputMode(InputModeData);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	URPGInputComponent* RPGInputComponent = CastChecked<URPGInputComponent>(InputComponent);
	
	RPGInputComponent->BindTaggedAction(InputConfig,this,&ThisClass::OnInputTagPressed,&ThisClass::OnInputTagReleased,&ThisClass::OnInputTagHeld);
}

void ARPGPlayerController::ShowDamageNumber_Implementation(ACharacter* TargetCharacter, float DamageAmount, bool bBlockedHit, bool bCriticalHit)
{
	if (!IsValid(TargetCharacter) || !DamageTextPool) return;

	// Defer to next tick: Acquired() → OnRegister needs one frame for Slate
	// to fully sync the widget before PlayAnimation fires. Remote clients
	// already have this delay naturally via the RPC network round-trip.
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([this, DamageAmount, bBlockedHit, bCriticalHit,
			WorldLocation = TargetCharacter->GetActorLocation() + FVector(0, 0, 100),
			WeakTarget = MakeWeakObjectPtr(TargetCharacter)](float)
		{
			if (!WeakTarget.IsValid() || !DamageTextPool) return false;

			ADamageTextActor* TextActor = DamageTextPool->Acquire(WorldLocation);
			if (TextActor)
			{
				TextActor->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
			}
			return false;
		}), 0.0f);
}

void ARPGPlayerController::OnInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	ProcessInputTag(InputTag,ERPGInputEvent::IE_Pressed);
}

void ARPGPlayerController::OnInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	ProcessInputTag(InputTag,ERPGInputEvent::IE_Released);
}

void ARPGPlayerController::OnInputTagHeld(const FInputActionValue& InputActionValue,FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	ProcessInputTag(InputTag,ERPGInputEvent::IE_Held,InputActionValue);
}

void ARPGPlayerController::ProcessInputTag(FGameplayTag InputTag, ERPGInputEvent EventType,
                                            const FInputActionValue& InputActionValue)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessInputTag);
	if (IInputInteractable* Interface = Cast<IInputInteractable>(GetPawn()))
	{
		if (Interface->HandleNativeInput(InputTag,EventType,InputActionValue)) return;
	}

	if (GetASC())
	{
		if (EventType == ERPGInputEvent::IE_Pressed) GetASC()->AbilityInputTagPressed(InputTag);
		if (EventType == ERPGInputEvent::IE_Released) GetASC()->AbilityInputTagReleased(InputTag);
		if (EventType == ERPGInputEvent::IE_Held) GetASC()->AbilityInputTagHeld(InputTag);
	}
}

URPGAbilitySystemComponent* ARPGPlayerController::GetASC()
{
	if (AbilitySystemComponent == nullptr)
	{
		AbilitySystemComponent = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AbilitySystemComponent;
}
