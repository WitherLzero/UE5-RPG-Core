// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
/**
 * 
 */

struct RPGCORE_API FRPGGameplayTags
{
public:
	static const FRPGGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();
	
	/*
	 *  Attribute Tags
	 */
	FGameplayTag Attributes_Vital_HealthRegeneration;
	FGameplayTag Attributes_Vital_ManaRegeneration;
	FGameplayTag Attributes_Vital_MaxHealth;
	FGameplayTag Attributes_Vital_MaxMana;
	
	FGameplayTag Attributes_Meta_IncomingXP;
	
	
	/*
	 *  Abilities
	 */
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_HitReact;
	
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;
    
	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	
	/*
	 *  Debuff Tags
	 */
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Duration;
	FGameplayTag Debuff_Frequency;
	
	
	/*
	 *  Event Tags
	 */
	FGameplayTag Event_Character_Died;
	FGameplayTag Event_Action_UpdateWarpingTarget;
	
	
	/* 
	 * State Tags
	 */
	FGameplayTag State_Stunned;
	FGameplayTag State_Action_Targeting;
	
	/*
	 *  Input Tags
	 */
	FGameplayTag Inputs_LMB;
	FGameplayTag Inputs_RMB;
	FGameplayTag Inputs_1;
	FGameplayTag Inputs_2;
	FGameplayTag Inputs_3;
	FGameplayTag Inputs_4;
	FGameplayTag Inputs_Move;
	
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	
	
	/*
	 *  Effect Tags
	 */
	FGameplayTag Effects_HitReact;
	
	
	/*
	*  Montage
	*/
	FGameplayTag Montage_Attack_Weapon;
	FGameplayTag Montage_Attack_LeftHand;
	FGameplayTag Montage_Attack_RightHand;
	FGameplayTag Montage_Attack_Tail;
	

	
private:
	static FRPGGameplayTags GameplayTags;
};
