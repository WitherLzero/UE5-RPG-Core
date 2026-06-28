// Copyright rynnli

#include "EditorTools/RPGEditorFunctionLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorTools/RPGEditorListItemData.h"
#include "Engine/Blueprint.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "UObject/UnrealType.h"

namespace
{
	bool ContainsAnyGameplayTag(const UObject* InObject, const TSet<FString>& TagStrings)
	{
		if (!InObject)
		{
			return false;
		}

		const UStruct* Struct = InObject->GetClass();
		const void* StructValue = InObject;

		for (TPropertyValueIterator<FStructProperty> It(Struct, StructValue); It; ++It)
		{
			const FStructProperty* StructProp = It->Key;
			const void* ValuePtr = It->Value;

			if (!StructProp || !ValuePtr)
			{
				continue;
			}

			if (StructProp->Struct == FGameplayTag::StaticStruct())
			{
				const FGameplayTag* Tag = static_cast<const FGameplayTag*>(ValuePtr);
				if (Tag && TagStrings.Contains(Tag->ToString()))
				{
					return true;
				}
			}
			else if (StructProp->Struct == FGameplayTagContainer::StaticStruct())
			{
				const FGameplayTagContainer* TagContainer = static_cast<const FGameplayTagContainer*>(ValuePtr);
				if (TagContainer)
				{
					TArray<FGameplayTag> Tags;
					TagContainer->GetGameplayTagArray(Tags);
					for (const FGameplayTag& Tag : Tags)
					{
						if (TagStrings.Contains(Tag.ToString()))
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool ShouldSkipAssetClass(const FTopLevelAssetPath& AssetClassPath)
	{
		static const TSet<FTopLevelAssetPath> SkippedClasses = {
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("World"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("Texture2D"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("TextureCube"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("StaticMesh"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("SkeletalMesh"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("SoundWave"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("SoundCue"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("Material"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("MaterialInstanceConstant"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("MaterialInstanceDynamic"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("Font"))),
			FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("FontFace"))),
		};

		return SkippedClasses.Contains(AssetClassPath);
	}
}

TArray<FString> URPGEditorFunctionLibrary::GetAllGameplayTagStrings()
{
	TArray<FString> Result;

	FGameplayTagContainer Container;
	UGameplayTagsManager::Get().RequestAllGameplayTags(Container, false);

	TArray<FGameplayTag> TagArray;
	Container.GetGameplayTagArray(TagArray);

	Result.Reserve(TagArray.Num());
	for (const FGameplayTag& Tag : TagArray)
	{
		Result.Add(Tag.ToString());
	}

	return Result;
}

int32 URPGEditorFunctionLibrary::GetGameplayTagCount()
{
	FGameplayTagContainer Container;
	UGameplayTagsManager::Get().RequestAllGameplayTags(Container, false);
	return Container.Num();
}

TArray<FString> URPGEditorFunctionLibrary::FindAssetsReferencingGameplayTags(const TArray<FString>& TagStrings)
{
	TArray<FString> Result;
	if (TagStrings.IsEmpty())
	{
		return Result;
	}

	const TSet<FString> TagSet(TagStrings);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByPath(FName(TEXT("/Game")), AssetDataList, /*bRecursive=*/true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (ShouldSkipAssetClass(AssetData.AssetClassPath))
		{
			continue;
		}

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			continue;
		}

		bool bMatches = false;
		if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			if (Blueprint->GeneratedClass)
			{
				UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
				bMatches = ContainsAnyGameplayTag(CDO, TagSet);
			}
		}
		else
		{
			bMatches = ContainsAnyGameplayTag(Asset, TagSet);
		}

		if (bMatches)
		{
			Result.Add(AssetData.GetObjectPathString());
		}
	}

	return Result;
}

TArray<URPGEditorListItemData*> URPGEditorFunctionLibrary::FindAssetListItemsReferencingGameplayTags(const TArray<FString>& TagStrings)
{
	TArray<URPGEditorListItemData*> Result;

	const TArray<FString> MatchingPaths = FindAssetsReferencingGameplayTags(TagStrings);
	Result.Reserve(MatchingPaths.Num());

	for (const FString& Path : MatchingPaths)
	{
		URPGEditorListItemData* Item = NewObject<URPGEditorListItemData>(GetTransientPackage());
		Item->DisplayString = Path;
		Result.Add(Item);
	}

	return Result;
}
