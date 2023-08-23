// Copyright Epic Games, Inc. All Rights Reserved.

#include "arcweave.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "arcweaveLibrary/ExampleLibrary.h"
#include "antlr4/Antlr4Library.h"

#define LOCTEXT_NAMESPACE "FarcweaveModule"

void FarcweaveModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("arcweave")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/arcweaveLibrary/Win64/ExampleLibrary.dll"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/arcweaveLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/arcweaveLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	// Add on the relative location of the third party dll and load it
	FString Antlr4LibraryPath;
#if PLATFORM_WINDOWS
	Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/antlr4/Win64/antlr4-runtime.dll"));
#elif PLATFORM_MAC
	//Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/arcweaveLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	//Antlr4LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/arcweaveLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (ExampleLibraryHandle)
	{
		// Call the test function in the third party library that opens a message box
		ExampleLibraryFunction();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}

	Antlr4LibraryHandle = !Antlr4LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*Antlr4LibraryPath) : nullptr;

	if (Antlr4LibraryHandle)
	{
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

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
	FPlatformProcess::FreeDllHandle(Antlr4LibraryHandle);
	ExampleLibraryHandle = nullptr;
	Antlr4LibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FarcweaveModule, arcweave)
