// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/GAS/Abilities/DamageGameplayAbility.h"
#include "RPGBeamSpell.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORE_API URPGBeamSpell : public UDamageGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(AActor* OriginActor, TArray<AActor*>& OutAdditionalTargets);
protected:
	UPROPERTY(BlueprintReadWrite)
	FVector MouseHitLocation;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> MouseHitActor;

	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxNumShockTargets = 5;
};
