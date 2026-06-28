// Copyright rynnli

#include "GameplayTagBrowser/RPGCoreEditorFunctionLibrary.h"

#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "SGameplayTagPicker.h"
#include "Widgets/SWindow.h"

TArray<FString> URPGCoreEditorFunctionLibrary::OpenGameplayTagPicker(const TArray<FString>& InitialTags, const FString& Filter)
{
	TArray<FString> Result;

	if (!GEditor)
	{
		return Result;
	}

	FGameplayTagContainer InitialContainer;
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	for (const FString& TagString : InitialTags)
	{
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound=*/false);
		if (Tag.IsValid())
		{
			InitialContainer.AddTag(Tag);
		}
	}

	TSharedPtr<FGameplayTagContainer> SelectedContainer = MakeShared<FGameplayTagContainer>(InitialContainer);

	SGameplayTagPicker::FOnTagChanged OnChanged =
		SGameplayTagPicker::FOnTagChanged::CreateLambda([SelectedContainer](const TArray<FGameplayTagContainer>& TagContainers)
		{
			if (TagContainers.Num() > 0)
			{
				*SelectedContainer = TagContainers[0];
			}
		});

	TArray<FGameplayTagContainer> EditableContainers;
	EditableContainers.Add(InitialContainer);

	TSharedPtr<SWindow> PickerWindow = SNew(SWindow)
		.Title(NSLOCTEXT("RPGCoreEditor", "SelectGameplayTags", "Select Gameplay Tags"))
		.ClientSize(FVector2D(500, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	PickerWindow->SetContent(
		SNew(SGameplayTagPicker)
			.TagContainers(EditableContainers)
			.Filter(Filter)
			.MultiSelect(true)
			.MaxHeight(0.0f)
			.OnTagChanged(OnChanged)
	);

	GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());

	TArray<FGameplayTag> SelectedTags;
	SelectedContainer->GetGameplayTagArray(SelectedTags);
	for (const FGameplayTag& Tag : SelectedTags)
	{
		Result.Add(Tag.ToString());
	}

	return Result;
}

void URPGCoreEditorFunctionLibrary::OpenReferenceViewerForGameplayTags(const TArray<FString>& TagStrings, bool bIncludeChildren)
{
	if (!FEditorDelegates::OnOpenReferenceViewer.IsBound())
	{
		return;
	}

	TSet<FName> UniqueTagNames;
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	auto AddTagAndMaybeChildren = [&UniqueTagNames, &Manager, bIncludeChildren](const FString& TagString)
	{
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound=*/false);
		if (!Tag.IsValid())
		{
			return;
		}

		UniqueTagNames.Add(Tag.GetTagName());

		if (bIncludeChildren)
		{
			FGameplayTagContainer Children = Manager.RequestGameplayTagChildren(Tag);
			TArray<FGameplayTag> ChildTags;
			Children.GetGameplayTagArray(ChildTags);
			for (const FGameplayTag& ChildTag : ChildTags)
			{
				UniqueTagNames.Add(ChildTag.GetTagName());
			}
		}
	};

	for (const FString& TagString : TagStrings)
	{
		AddTagAndMaybeChildren(TagString);
	}

	if (UniqueTagNames.IsEmpty())
	{
		return;
	}

	TArray<FAssetIdentifier> AssetIdentifiers;
	AssetIdentifiers.Reserve(UniqueTagNames.Num());
	for (const FName& TagName : UniqueTagNames)
	{
		AssetIdentifiers.Emplace(FGameplayTag::StaticStruct(), TagName);
	}

	FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers, FReferenceViewerParams());
}
