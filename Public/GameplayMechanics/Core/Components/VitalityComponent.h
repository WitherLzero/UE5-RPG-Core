// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "VitalityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathSignature, AActor*, DeadActor, AActor*, Instigator);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGCORE_API UVitalityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVitalityComponent();

	UFUNCTION(BlueprintCallable, Category = "Vitality")
	void InitVitality(UAbilitySystemComponent* ASC);
	
	UPROPERTY(BlueprintAssignable, Category = "Vitality")
	FOnDeathSignature OnDeath;
	
	FORCEINLINE bool IsDead() const { return bIsDead; }
protected:
	virtual void BeginPlay() override;
	
	void HandleOutOfHealth(AActor* Instigator);
	
	void Die();
	
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();
	
	void PerformRagdoll();
	
	bool bIsDead = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* DeathSound;
};
