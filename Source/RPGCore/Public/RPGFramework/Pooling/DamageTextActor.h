// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageTextActor.generated.h"

class UWidgetComponent;
class UDamageTextPoolManager;

/**
 * Floating damage number Actor for object pooling.
 *
 * Pooling invariant: Actor::IsHidden() is ALWAYS false throughout its lifecycle.
 * Visibility is controlled entirely through WidgetComponent::SetHiddenInGame(),
 * so that UWidgetComponent::UpdateWidgetOnScreen() never fails the
 * !(GetOwner()->IsHidden()) check.
 *
 * Lifecycle:
 *   PreWarmed  → WidgetComp Hidden,  Actor visible, in AvailablePool
 *   Acquired   → WidgetComp Visible, Actor visible, in ActivePool
 *   Released   → WidgetComp Hidden,  Actor visible, in AvailablePool
 */
UCLASS(Blueprintable)
class RPGCORE_API ADamageTextActor : public AActor
{
	GENERATED_BODY()

public:
	ADamageTextActor();

	/** Set damage values (BP implementable, called by pool). */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetDamageText(float Damage, bool bBlockedHit, bool bCriticalHit);

	/** Called when acquired from pool. Restores widget visibility. */
	void Acquired();

	/** Called when released back to pool. Hides widget only. */
	void Released();

	/** Store a reference to the owning pool manager. */
	void SetPoolManager(UDamageTextPoolManager* InManager) { PoolManager = InManager; }

	/** Owning pool manager. */
	UFUNCTION(BlueprintPure)
	UDamageTextPoolManager* GetPoolManager() const { return PoolManager; }

	/** The WidgetComponent that renders the damage text. */
	UFUNCTION(BlueprintPure)
	UWidgetComponent* GetDamageTextWidget() const { return DamageTextComp; }

protected:
	/** WidgetComponent — renders the damage text widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageText")
	TObjectPtr<UWidgetComponent> DamageTextComp;

	/** Root scene component. */
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

private:
	/** Owning pool manager (weak back-reference for Release). */
	UPROPERTY()
	TObjectPtr<UDamageTextPoolManager> PoolManager;
};
