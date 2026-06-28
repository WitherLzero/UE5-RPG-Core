// Copyright rynnli

#include "RPGCore/RPGCore.h"
#include "Modules/ModuleManager.h"
#include "RPGFramework/Types/RPGGameplayTags.h"

class FRPGCoreModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		
		FRPGGameplayTags::InitializeNativeGameplayTags();
	}
};

IMPLEMENT_MODULE(FRPGCoreModule, RPGCore)
