// Copyright rynnli

using UnrealBuildTool;

public class RPGCoreEditor : ModuleRules
{
	public RPGCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "RPGCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"Slate", "SlateCore",
			"GameplayTags",
			"GameplayTagsEditor",
			"AssetRegistry",
			"LevelEditor",
			"ToolMenus",
			"Blutility",
			"UMGEditor"
		});
	}
}
