// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

/**
 * WidgetComponent for displaying floating damage numbers.
 *
 * Retained for backward compatibility. The new Actor-pooling system
 * (ADamageTextActor + UDamageTextPoolManager) uses a raw UWidgetComponent
 * directly without subclassing this class.
 */
UCLASS()
class RPGCORE_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(const float& Damage, bool bBlockedHit, bool bCriticalHit);
};
