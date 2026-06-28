// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGCoreEditorFunctionLibrary.generated.h"

/**
 * Editor-only utility functions for RPGCore editor tools.
 * Designed to be called from Editor Utility Widgets / Blutilities.
 */
UCLASS()
class RPGCOREEDITOR_API URPGCoreEditorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Opens the native GameplayTag picker as a modal window and returns the selected tag names. */
	UFUNCTION(BlueprintCallable, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static TArray<FString> OpenGameplayTagPicker(const TArray<FString>& InitialTags, const FString& Filter = TEXT(""));

	/** Opens the native Reference Viewer with the given GameplayTag names as graph roots. */
	UFUNCTION(BlueprintCallable, Category = "RPGCore|EditorTools|GameplayTags", meta = (DevelopmentOnly))
	static void OpenReferenceViewerForGameplayTags(const TArray<FString>& TagStrings, bool bIncludeChildren = false);
};
