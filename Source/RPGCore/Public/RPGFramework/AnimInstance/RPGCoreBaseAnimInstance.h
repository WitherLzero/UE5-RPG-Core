// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RPGCoreBaseAnimInstance.generated.h"

/**
 * 通用 AnimInstance 基类,所有项目的动画实例共享。
 * 定位与课程仓库 UWarriorBaseAnimInstance 的空壳基类一致,未来可放置通用动画工具。
 */
UCLASS()
class RPGCORE_API URPGCoreBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
};