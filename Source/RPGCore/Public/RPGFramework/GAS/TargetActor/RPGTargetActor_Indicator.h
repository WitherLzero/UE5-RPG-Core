// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "RPGTargetActor_Indicator.generated.h"

class UDecalComponent;

/**
 * Ground-targeting indicator with a decal ring for two-stage activation abilities. */
UCLASS()
class RPGCORE_API ARPGTargetActor_Indicator : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ARPGTargetActor_Indicator();

	// Called by the ability task to begin targeting.
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	// Routed through internal delegates when the ASC fires LocalInputConfirm.
	virtual void ConfirmTargetingAndContinue() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Decal component that visualises the indicator radius on the ground.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Indicator")
	TObjectPtr<UDecalComponent> DecalComp;

	// Cursor trace cached by Tick so ConfirmTargetingAndContinue does not re-query.
	FHitResult CachedHitResult;

};
