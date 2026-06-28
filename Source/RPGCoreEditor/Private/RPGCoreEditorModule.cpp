// Copyright rynnli

#include "RPGCoreEditorModule.h"

#include "ToolMenus.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "FRPGCoreEditorModule"

static const FName RPGCoreToolMenuName("LevelEditor.MainMenu.Tools");

void FRPGCoreEditorModule::StartupModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	// Register an owner so our entries are cleaned up on module shutdown
	FToolMenuOwnerScoped OwnerScoped(this);

	// Extend the existing Tools menu in the Level Editor main menu bar
	UToolMenu* ToolsMenu = ToolMenus->ExtendMenu(RPGCoreToolMenuName);
	if (!ToolsMenu)
	{
		return;
	}

	// Add a section for RPGCore tools
	FToolMenuSection& Section = ToolsMenu->AddSection(
		"RPGCoreTools",
		LOCTEXT("RPGCoreToolsSection", "RPGCore")
	);

	Section.AddMenuEntry(
		"GameplayTagBrowser",
		LOCTEXT("GameplayTagBrowserLabel", "GameplayTag Browser"),
		LOCTEXT("GameplayTagBrowserTooltip", "Open the GameplayTag Reference Browser"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FRPGCoreEditorModule::OpenGameplayTagBrowser))
	);
}

void FRPGCoreEditorModule::ShutdownModule()
{
}

void FRPGCoreEditorModule::OpenGameplayTagBrowser()
{
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	if (!EditorUtilitySubsystem)
	{
		return;
	}

	const FSoftObjectPath EUWPath(TEXT("/RPGCore/EditorUtilities/GameplayTagBrowser/EUW_GameplayTagBrowser.EUW_GameplayTagBrowser"));
	UObject* EUWObject = EUWPath.TryLoad();
	if (UEditorUtilityWidgetBlueprint* EUWBP = Cast<UEditorUtilityWidgetBlueprint>(EUWObject))
	{
		EditorUtilitySubsystem->SpawnAndRegisterTab(EUWBP);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRPGCoreEditorModule, RPGCoreEditor)
