// Copyright rynnli

#include "RPGFramework/GAS/TargetActor/RPGTargetActor_Indicator.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "GameFramework/PlayerController.h"

ARPGTargetActor_Indicator::ARPGTargetActor_Indicator()
{
	// 允许每帧 Tick 来追踪鼠标
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	
	DecalComp->DecalSize = FVector(10.f, 200.f, 200.f); 
	DecalComp->SetVisibility(false); // 创建时隐藏，StartTargeting 时显示

	// 非常重要：鼠标坐标采集只在拥有鼠标的本地客户端进行
	ShouldProduceTargetDataOnServer = false;
}

void ARPGTargetActor_Indicator::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	
	// 技能正式开始索敌，把魔法阵显示出来
	DecalComp->SetVisibility(true);
}

void ARPGTargetActor_Indicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 不断向 PlayerController 索要底板坐标，移动自身
	if (APlayerController* PC =  OwningAbility->GetActorInfo().PlayerController.Get())
	{
		FHitResult HitResult;
		// 此处使用 ECC_Visibility 碰撞通道，实际可换成专属的 Floor TraceChannel
		PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

		if (HitResult.bBlockingHit)
		{
			SetActorLocation(HitResult.Location);
		}
	}
}

void ARPGTargetActor_Indicator::ConfirmTargetingAndContinue()
{
	// 确保是该由本地端产生数据（有鼠标那一端）
	if (IsConfirmTargetingAllowed() && ShouldProduceTargetData())
	{
		FHitResult HitResult;
		if (APlayerController* PC = OwningAbility->GetActorInfo().PlayerController.Get())
		{
			PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		}

		// 1. 组装 GAS 专用的目标数据结构
		FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
		LocationData->TargetLocation.LiteralTransform = FTransform(HitResult.Location);
		LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		
		FGameplayAbilityTargetDataHandle DataHandle;
		DataHandle.Add(LocationData);

		// 2. 本地广播数据，WaitTargetData 节点收到后会继续执行
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
	
}
