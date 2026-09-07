// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "UObject/ScriptInterface.h"
#include "CursorHighlightComponent.generated.h"

class APlayerController;
class IEnemyInterface;

/**
 * Cursor-driven enemy highlight: a throttled trace under the cursor drives
 * IEnemyInterface Highlight/UnHighlight on hovered-actor transitions.
 *
 * Separated from ARPGPlayerController (see docs/adr/0003): cursor-driven
 * targeting is a top-down mechanic, not a controller genre assumption.
 * Mount on the Pawn; the component self-ticks and is inert for hosts that
 * never mount it. The trace can be blocked at runtime via the
 * Player.Block.CursorTrace GameplayTag on the owner's ASC.
 */
UCLASS(ClassGroup=(RPGCore), meta=(BlueprintSpawnableComponent))
class RPGCORE_API UCursorHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCursorHighlightComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Actor currently hovered by the cursor, or nullptr. */
	IEnemyInterface* GetHoveredEnemy() const { return ThisActor.GetInterface(); }

protected:
	/** Trace rate limit (Hz). 30 Hz keeps highlight latency unnoticeable. */
	UPROPERTY(EditDefaultsOnly, Category="CursorHighlight")
	float TraceRate = 30.f;

	UPROPERTY(EditDefaultsOnly, Category="CursorHighlight")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	void CursorTrace();
	bool IsTraceBlocked() const;
	APlayerController* ResolveController() const;

	TWeakObjectPtr<APlayerController> CachedPC;
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;
	float LastTraceTime = 0.f;
};
