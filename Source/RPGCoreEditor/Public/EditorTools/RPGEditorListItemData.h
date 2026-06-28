// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGEditorListItemData.generated.h"

/**
 * Generic UObject wrapper used to pass simple data (e.g. asset paths, strings)
 * into UMG ListViews / Editor Utility Widgets that require UObject items.
 */
UCLASS(BlueprintType)
class RPGCOREEDITOR_API URPGEditorListItemData : public UObject
{
	GENERATED_BODY()

public:
	/** Primary payload string, e.g. /Game/Blueprints/... */
	UPROPERTY(BlueprintReadWrite, Category = "List Item Data")
	FString DisplayString;

	/** Optional secondary string, e.g. asset class or extra description. */
	UPROPERTY(BlueprintReadWrite, Category = "List Item Data")
	FString SecondaryString;
};
