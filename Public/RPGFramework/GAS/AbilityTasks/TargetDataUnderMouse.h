// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataDelegate, const FGameplayAbilityTargetDataHandle&,DataHandle);
/**
 * 
 */
UCLASS()
class RPGCORE_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable,Category="Ability|Tasks", 
		meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwingAbility", DefaultToSelf = "OwingAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwingAbility);
	
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataDelegate ValidData;
	
private:
	virtual void Activate() override;
	void SendMouseCursorData();
	void OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
