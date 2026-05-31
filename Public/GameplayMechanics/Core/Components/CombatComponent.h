// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGCORE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	void RegisterWeaponMesh(USceneComponent* InMesh);
	
	UFUNCTION(BlueprintCallable)
	FVector GetCombatSocketLocation(FGameplayTag MontageTag) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetHitReactEffect();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCombatTarget(AActor* InTarget) { CombatTarget = InTarget; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE AActor* GetCombatTarget() const { return CombatTarget; }	

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UNiagaraSystem> HitReactEffect;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> WeaponMesh;
	
	UPROPERTY(EditAnywhere,Category= "Combat")
	TObjectPtr<class UCombatConfig> CombatConfig;
	
	UPROPERTY(EditAnywhere,Category= "Combat")
	TArray<FTaggedMontage> AttackMontages;
	
	UPROPERTY(VisibleAnywhere,Category= "Combat")
	AActor* CombatTarget;
	
};
