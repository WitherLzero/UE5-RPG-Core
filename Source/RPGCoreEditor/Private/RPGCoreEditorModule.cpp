// Copyright rynnli

#include "RPGCoreEditorModule.h"

#define LOCTEXT_NAMESPACE "FRPGCoreEditorModule"

void FRPGCoreEditorModule::StartupModule()
{
	// No eager initialization needed; BlueprintCallable functions are bound on demand.
}

void FRPGCoreEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRPGCoreEditorModule, RPGCoreEditor)
