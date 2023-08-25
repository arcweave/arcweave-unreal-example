// Copyright Epic Games, Inc. All Rights Reserved.

#include "arcweave.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "ArcscriptTranspiler.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogArcwarePlugin);

#define LOCTEXT_NAMESPACE "FarcweaveModule"

void FarcweaveModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	//FString BaseDir = IPluginManager::Get().FindPlugin("arcweave")->GetBaseDir();
	UE_LOG(LogArcwarePlugin, Warning, TEXT("Arcware plugin module started!"));

	// Add on the relative location of the third party dll and load it
	FString BaseDir = IPluginManager::Get().FindPlugin("arcweave")->GetBaseDir();
	FString Antlr4LibraryPath;
#if PLATFORM_WINDOWS
	Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/antlr4/Win64/antlr4-runtime.dll"));
#elif PLATFORM_MAC
	//Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/arcweaveLibrary/Mac/Release/libExampleLibrary.dylib"));
	#elif PLATFORM_LINUX
	//Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/arcweaveLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
	#endif // PLATFORM_WINDOWS

	Antlr4LibraryHandle = !Antlr4LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*Antlr4LibraryPath) : nullptr;

	if (Antlr4LibraryHandle)
	{
		TestJsonFile();
		// Call the test function in the third party library that opens a message box
		//Antlr4LibraryFunction();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("Antlr4LibraryError", "Failed to load antlr library"));
	}

}

void FarcweaveModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogArcwarePlugin, Warning, TEXT("Arcware plugin module shutdown!"));
	// Free the dll handle
	FPlatformProcess::FreeDllHandle(Antlr4LibraryHandle);
	Antlr4LibraryHandle = nullptr;
}

bool FarcweaveModule::TestJsonFile()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("arcweave")->GetBaseDir();
	FString JsonFilePath = FPaths::Combine(*BaseDir, TEXT("Source/arcweave/test/valid.json"));
	FString JsonRaw;

	// Load the file to a FString
	if (!FFileHelper::LoadFileToString(JsonRaw, *JsonFilePath))
	{
		UE_LOG(LogArcwarePlugin, Error, TEXT("Failed to load JSON file!"));
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TSharedPtr<FJsonObject>* InitialVarsObject;
		if (JsonObject->TryGetObjectField("initialVars", InitialVarsObject))
		{
			for (const auto& Pair : (*InitialVarsObject)->Values)
			{
				TSharedPtr<FJsonObject> VarObject = Pair.Value->AsObject();
				if (VarObject.IsValid() && VarObject->HasField("name"))
				{
					FString NameValue = VarObject->GetStringField("name");
					if (NameValue == "x")
					{
						UE_LOG(LogTemp, Warning, TEXT("Test passed!"));
						return true;
					}
				}
			}
		}
	}

	UE_LOG(LogArcwarePlugin, Warning, TEXT("Test failed!"));
	return false;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FarcweaveModule, arcweave)
