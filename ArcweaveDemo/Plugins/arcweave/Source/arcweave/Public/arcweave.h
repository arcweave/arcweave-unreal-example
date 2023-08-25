// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogArcwarePlugin, Log, All);

class FarcweaveModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool TestJsonFile();

private:
	/** Handle to the test antlr dll we will load */
	void*	Antlr4LibraryHandle;	

};
