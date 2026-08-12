// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/AnimInstance/RPGCoreBaseAnimInstance.h"
#include "RPGCoreLinkedAnimLayer.generated.h"

class URPGCoreCharacterAnimInstance;

/**
 * 通用 Linked Anim Layer 基类,为所有项目的动画层提供到主 AnimInstance 的桥接。
 *
 * Linked Anim Layer 是独立实例化的 UAnimInstance 子类,它不知道主 ABP
 * (通常是 URPGCoreCharacterAnimInstance 派生类)上算好的数据。
 * 本类提供 GetCharacterAnimInstance(),从 OwningComponent 反查主实例并 Cast 成核心类型。
 */
UCLASS(Abstract)
class RPGCORE_API URPGCoreLinkedAnimLayer : public URPGCoreBaseAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	URPGCoreCharacterAnimInstance* GetCharacterAnimInstance() const;
};