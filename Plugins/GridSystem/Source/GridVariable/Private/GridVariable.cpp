// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridVariable.h"
#include "GridDataUIChanger.h"

#define LOCTEXT_NAMESPACE "FGridVariableModule"

void FGridVariableModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"GridData",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&GridDataUIChanger::MakeInstance)
	);
}

void FGridVariableModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGridVariableModule, GridSystem)