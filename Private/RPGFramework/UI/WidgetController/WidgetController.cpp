// Copyright rynnli


#include "RPGFramework/UI/WidgetController/WidgetController.h"

#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/GAS/Data/AbilityInfo.h"

void UWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
}

void UWidgetController::BroadcastInitialValues()
{
}

void UWidgetController::BindCallbacksToDependencies()
{
}

void UWidgetController::OnInitializeStartupAbilities(URPGAbilitySystemComponent* ASC)
{
	if (!ASC->bStartupAbilitiesGiven) return;
	
	FAbilitySpecAction GetAbilityInfo;
	GetAbilityInfo.BindLambda(
		[this,ASC](const FGameplayAbilitySpec& Spec)
		{
			FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(ASC->GetAbilityTagFromSpec(Spec));
			Info.InputTag = ASC->GetInputTagFromSpec(Spec);
			Info.StatusTag = ASC->GetStatusFromSpec(Spec);
			OnAbilityInfoGet.Broadcast(Info);
		});
	ASC->ApplyActionToAbilities(GetAbilityInfo);
}
