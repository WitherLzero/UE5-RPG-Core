// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "DamageTextPoolManager.generated.h"

class ADamageTextActor;
class APlayerController;

/**
 * Object pool manager for ADamageTextActor instances.
 *
 * Prewarms a fixed number of actors and reuses them via Acquire/Release.
 * Actors are spawned in the persistent world and are never hidden —
 * only their WidgetComponent visibility is toggled.
 *
 * Lifecycle:
 *   PreWarm  → Spawn N actors, keep them hidden in AvailablePool.
 *   Acquire  → Pop from pool (or Spawn new), SetActorLocation, show widget.
 *   Release  → Hide widget, return to pool for reuse.
 */
UCLASS()
class RPGCORE_API UDamageTextPoolManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(APlayerController* Owner, TSubclassOf<ADamageTextActor> InClass);

	UFUNCTION(BlueprintCallable)
	void PreWarm(int32 Count);

	/** Acquire a damage text actor from the pool and place it at WorldLocation. */
	UFUNCTION(BlueprintCallable)
	ADamageTextActor* Acquire(const FVector& WorldLocation);

	/** Return an actor to the pool. */
	UFUNCTION(BlueprintCallable)
	void Release(ADamageTextActor* Actor);

	void Shutdown();

private:
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerController;

	UPROPERTY()
	TSubclassOf<ADamageTextActor> DamageTextActorClass;

	UPROPERTY()
	TArray<TObjectPtr<ADamageTextActor>> AvailablePool;

	UPROPERTY()
	TArray<TObjectPtr<ADamageTextActor>> ActivePool;

	int32 MaxPoolSize = 32;
};
