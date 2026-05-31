// Copyright rynnli


#include "RPGFramework/Player/RPGPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGFramework/Character/RPGCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/Input/RPGInputComponent.h"
#include "RPGFramework/Interaction/EnemyInterface.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "RPGFramework/UI/Widgets/DamageTextComponent.h"


ARPGPlayerController::ARPGPlayerController()
{
	bReplicates = true;
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
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
	
	UpdateMouse();
}

void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	URPGInputComponent* RPGInputComponent = CastChecked<URPGInputComponent>(InputComponent);
	
	RPGInputComponent->BindTaggedAction(InputConfig,this,&ThisClass::OnInputTagPressed,&ThisClass::OnInputTagReleased,&ThisClass::OnInputTagHeld);
}

void ARPGPlayerController::ShowDamageNumber_Implementation(ACharacter* TargetCharacter, float DamageAmount, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextCompClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter,DamageTextCompClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount,bBlockedHit,bCriticalHit);
	}
}

void ARPGPlayerController::CursorTrace()
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_CursorTrace))
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->UnHighlightActor();
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
}


void ARPGPlayerController::UpdateMouse()
{
	// TODO: Specific cursor setting for Top-Down? Need to make it changeable 
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
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

bool ARPGPlayerController::GetCursorHit(FHitResult& HitResult)
{
	return GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
}


