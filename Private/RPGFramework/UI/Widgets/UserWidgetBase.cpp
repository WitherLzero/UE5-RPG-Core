// Copyright rynnli


#include "RPGFramework/UI/Widgets/UserWidgetBase.h"

void UUserWidgetBase::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	OnWidgetControllerSet();
}
