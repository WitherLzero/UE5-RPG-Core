// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"


class UMotionWarpingComponent;
class UAbilitySystemComponent;
struct FGameplayEventData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGCORE_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionComponent();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void InitActionComponent(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable)
	void UpdateFacingTarget(const FVector& TargetLoc);
protected:
	virtual void BeginPlay() override;

	void OnUpdateWarpingTargetEvent(const FGameplayEventData* Payload);

	TObjectPtr<UMotionWarpingComponent> MotionWarpingComp;
	
	UPROPERTY(EditAnywhere, Category= "ARPG")
	FName FacingTargetName;
};
