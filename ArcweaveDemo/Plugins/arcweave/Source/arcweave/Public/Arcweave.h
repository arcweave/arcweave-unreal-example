// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Check if ARCWEAVE_API is not defined, then define it as DLL_IMPORT
#ifndef ARCWEAVE_API
#define ARCWEAVE_API DLL_IMPORT
#endif

#include <map>
#include <string>

#include "ArcscriptHelpers.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectMacros.h"

class FJsonObject;
DECLARE_LOG_CATEGORY_EXTERN(LogArcwarePlugin, Log, All);

class FArcweaveModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	UFUNCTION(BlueprintCallable, Category = "Arcweave | Functions")
	bool TestJsonFile();

private:
	/** Handle to the test antlr dll we will load */
	void*	Antlr4LibraryHandle;

	void TraspileArcscript();
	std::map<std::string, Variable> GetInitialVars(TSharedPtr<FJsonObject> JsonObject);
	std::string CompareResults(TSharedPtr<FJsonObject> expected, std::any actual);
	std::map<std::string, std::any> GetExpectedVars(TSharedPtr<FJsonObject> expectedVarsJson);
	std::string CompareVars(std::map<std::string, std::any> expected, std::map<std::string, std::any> actual);

};
