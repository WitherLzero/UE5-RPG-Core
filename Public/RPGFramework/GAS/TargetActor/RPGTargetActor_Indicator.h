// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "RPGTargetActor_Indicator.generated.h"

class UDecalComponent;

/**
 * 通用的面向地面的二段施法指示器（圆形拾取�? */
UCLASS()
class RPGCORE_API ARPGTargetActor_Indicator : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ARPGTargetActor_Indicator();

	// Task 节点开始阶段会调用此函数
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	// ASC 触发 LocalInputConfirm 时，通过内部委托会路由到这里
	virtual void ConfirmTargetingAndContinue() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// 用于显示范围圈的贴花组件
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Indicator")
	TObjectPtr<UDecalComponent> DecalComp;

};
