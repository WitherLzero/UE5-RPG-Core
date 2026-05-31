// Copyright rynnli


#include "RPGFramework/Types/RPGGameplayTags.h"
#include "GameplayTagsManager.h"

FRPGGameplayTags FRPGGameplayTags::GameplayTags;

void FRPGGameplayTags::InitializeNativeGameplayTags()
{

	GameplayTags.Attributes_Vital_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.HealthRegeneration"),
		FString("Amount of Health regenerated every 1 second")
		);

	GameplayTags.Attributes_Vital_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.ManaRegeneration"),
		FString("Amount of Mana regenerated every 1 second")
		);

	GameplayTags.Attributes_Vital_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxHealth"),
		FString("Maximum amount of Health obtainable")
		);

	GameplayTags.Attributes_Vital_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxMana"),
		FString("Maximum amount of Mana obtainable")
		);
	
	GameplayTags.Attributes_Meta_IncomingXP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Meta.IncomingXP"),
		FString("Incoming XP Meta Attribute")
		);	

	
	GameplayTags.Abilities_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.None"),
	FString("Ability Tag None ( nullptr )")
	);	
	
	GameplayTags.Abilities_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.HitReact"),
		FString("Hit React Ability")
		);
	
	GameplayTags.Abilities_Status_Eligible = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Eligible"),
		FString("Eligible Status")
		);

	GameplayTags.Abilities_Status_Equipped = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Equipped"),
		FString("Equipped Status")
		);

	GameplayTags.Abilities_Status_Locked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Locked"),
		FString("Locked Status")
		);

	GameplayTags.Abilities_Status_Unlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Unlocked"),
		FString("Unlocked Status")
		);	
	
	
	GameplayTags.Abilities_Type_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.None"),
		FString("Type None")
		);

	GameplayTags.Abilities_Type_Offensive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Offensive"),
		FString("Type Offensive")
		);

	GameplayTags.Abilities_Type_Passive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Passive"),
		FString("Type Passive")
		);	
	
	
	GameplayTags.Debuff_Chance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Chance"),
		FString("Debuff Chance")
		);
	GameplayTags.Debuff_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Damage"),
		FString("Debuff Damage")
		);
	GameplayTags.Debuff_Duration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Duration"),
		FString("Debuff Duration")
		);
	GameplayTags.Debuff_Frequency = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Frequency"),
		FString("Debuff Frequency")
		);		
	
	
	GameplayTags.Event_Character_Died = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Character.Died"),
	FString("Tag for sending Gameplay Event when character died.")
	);	

	GameplayTags.Event_Action_UpdateWarpingTarget = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Action.UpdateWarpingTarget"),
		FString("Event Tag to notify Action Component to update warping target")
		);
	
	
	GameplayTags.State_Stunned = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Stunned"),
FString("State Tag for stunned character.")
);

	GameplayTags.State_Action_Targeting = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Action.Targeting"),
		FString("State Tag for targeting phase.")
		);
	
	GameplayTags.Inputs_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.LMB"),
		FString("Input Tag for Left Mouse Button")
		);
	
	GameplayTags.Inputs_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.RMB"),
		FString("Input Tag for Right Mouse Button")
		);
	
	GameplayTags.Inputs_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.1"),
		FString("Input Tag for Key 1")
		);
	
	GameplayTags.Inputs_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.2"),
		FString("Input Tag for Key 2")
		);
	
	GameplayTags.Inputs_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.3"),
		FString("Input Tag for Key 3")
		);
	
	GameplayTags.Inputs_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.4"),
		FString("Input Tag for Key 4")
		);
	
	GameplayTags.Inputs_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Inputs.Move"),
		FString("Input Tag for Movement")
		);
	
	
	GameplayTags.Player_Block_CursorTrace = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Player.Block.CursorTrace"),
	FString("Block tracing under the cursor")
	);

	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"),
		FString("Block Input Held callback for input")
		);

	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"),
		FString("Block Input Pressed callback for input")
		);

	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"),
		FString("Block Input Released callback for input")
		);
	
	
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
FString("Effect Tag for HitReact")
	);
	
	/*
	 *  Montage
	 */
	
	GameplayTags.Montage_Attack_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.Weapon"),
		FString("Weapon")
		);
	
	GameplayTags.Montage_Attack_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.LeftHand"),
		FString("Left Hand")
		);
	
	GameplayTags.Montage_Attack_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.RightHand"),
		FString("Right Hand")
		);		
	
	GameplayTags.Montage_Attack_Tail = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.Tail"),
		FString("Tail")
		);			
}
