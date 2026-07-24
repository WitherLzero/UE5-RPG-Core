// Copyright rynnli

#include "RPGFramework/Pooling/DamageTextActor.h"
#include "Components/WidgetComponent.h"

ADamageTextActor::ADamageTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root scene component
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	// WidgetComponent — the actual damage text renderer
	DamageTextComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageText"));
	DamageTextComp->SetupAttachment(SceneRoot);
	DamageTextComp->SetWidgetSpace(EWidgetSpace::Screen);
	DamageTextComp->SetDrawSize(FVector2D(500.0f, 500.0f));
	DamageTextComp->SetPivot(FVector2D(0.5f, 0.5f));
	DamageTextComp->SetTickWhenOffscreen(true);

	// Actor starts fully visible; only WidgetComponent toggles hidden.
	// This ensures UWidgetComponent::UpdateWidgetOnScreen() never fails.
	bReplicates = false;
}

void ADamageTextActor::Acquired()
{
	// Activate but keep hidden.
	// SetDamageText (BP) will update text then show the widget.
	DamageTextComp->SetActive(true);
}

void ADamageTextActor::Released()
{
	DamageTextComp->SetActive(false);
	DamageTextComp->SetHiddenInGame(true);
}
