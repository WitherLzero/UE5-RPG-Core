// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "EditorTools/RPGEditorListItemData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGEditorFunctionLibrary.generated.h"

/**
 * Editor-only utility functions for RPGCore tools.
 * These are designed to be called from Editor Utility Widgets / Blutilities.
 */
UCLASS()
class RPGCOREEDITOR_API URPGEditorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns all registered GameplayTags as strings. */
	UFUNCTION(BlueprintCallable, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static TArray<FString> GetAllGameplayTagStrings();

	/** Returns the number of registered GameplayTags. */
	UFUNCTION(BlueprintPure, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static int32 GetGameplayTagCount();

	/** Finds all /Game assets that reference any of the provided GameplayTag strings (editor only). */
	UFUNCTION(BlueprintCallable, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static TArray<FString> FindAssetsReferencingGameplayTags(const TArray<FString>& TagStrings);

	/** Finds all /Game assets that reference any of the provided GameplayTag strings, returned as UObject list items for UMG ListViews. */
	UFUNCTION(BlueprintCallable, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static TArray<URPGEditorListItemData*> FindAssetListItemsReferencingGameplayTags(const TArray<FString>& TagStrings);
};
