// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/AnimInstance/RPGCoreBaseAnimInstance.h"
#include "RPGCoreCharacterAnimInstance.generated.h"

class ARPGCharacterBase;
class UCharacterMovementComponent;

/**
 * 通用角色动画实例基类。
 * 缓存 OwningCharacter / OwningMovementComponent,并计算通用 locomotion 数据
 * (GroundSpeed、bHasAcceleration),供 AnimGraph / Property Access / 派生类使用。
 */
UCLASS()
class RPGCORE_API URPGCoreCharacterAnimInstance : public URPGCoreBaseAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetGroundSpeed() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool GetHasAcceleration() const;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ARPGCharacterBase> OwningCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent> OwningMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float GroundSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bHasAcceleration = false;
};