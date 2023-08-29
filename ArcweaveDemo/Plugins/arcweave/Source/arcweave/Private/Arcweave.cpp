// Copyright Epic Games, Inc. All Rights Reserved.

#include "Arcweave.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "ArcscriptTranspiler.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogArcwarePlugin);

#define LOCTEXT_NAMESPACE "FarcweaveModule"

void FArcweaveModule::StartupModule()
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

void FArcweaveModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogArcwarePlugin, Warning, TEXT("Arcware plugin module shutdown!"));
	// Free the dll handle
	FPlatformProcess::FreeDllHandle(Antlr4LibraryHandle);
	Antlr4LibraryHandle = nullptr;
}

bool FArcweaveModule::TestJsonFile()
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
		std::map<std::string, Variable> initialVars;
		if (JsonObject->HasField("initialVars"))
		{
			initialVars = GetInitialVars(JsonObject);
		}
		
		const TArray<TSharedPtr<FJsonValue>>* CasesArray = nullptr;
        if (JsonObject->TryGetArrayField("cases", CasesArray))
        {
            for (TSharedPtr<FJsonValue> CaseValue : *CasesArray)
            {
				UE_LOG(LogArcwarePlugin, Log, TEXT("---- Case object ----"));

            	TSharedPtr<FJsonObject> CaseObject = CaseValue->AsObject();
				std::string code = TCHAR_TO_UTF8(*CaseObject->GetStringField("code"));
            	if (code.empty())
            	{
            		UE_LOG(LogArcwarePlugin, Error, TEXT("//// code field is empty, exiting ///// "));
            		return false;
            	}
            	UE_LOG(LogArcwarePlugin, Log, TEXT("Code: %s"), UTF8_TO_TCHAR(code.c_str()));
				std::map<std::string, int> currentVisits;
				std::string currentElement = "";

				if (CaseObject->HasField("elementId"))
				{
					currentElement = TCHAR_TO_UTF8(*CaseObject->GetStringField("elementId"));
					UE_LOG(LogArcwarePlugin, Log, TEXT("elementId %s"), *CaseObject->GetStringField("elementId"));
				}

				if (CaseObject->HasField("visits"))
				{
					TSharedPtr<FJsonObject> visitsObject = JsonObject->GetObjectField("visits");
    
					for (auto& VisitPair : visitsObject->Values)
					{
						FString Key = VisitPair.Key;
						int32 Value = 0; // Initialize with a default value
						Value = VisitPair.Value->AsNumber();
						currentVisits[TCHAR_TO_UTF8(*Key)] = Value;
						UE_LOG(LogTemp, Warning, TEXT("Key: %s, Value: %d"), *Key, Value);
					}
				}

				bool hasError = false;
				if (CaseObject->HasField("error"))
				{
					hasError = true;
				}

				// Initialize the transpiler (for now ignore the element ID)
				ArcscriptTranspiler transpiler(currentElement, initialVars, currentVisits);
				// Run the script in the Transpiler
				TranspilerOutput output;
				try {   // In case we have expected errors in file
					output =  transpiler.runScript(code);
				} catch(ParseErrorException&) {
					FString ErrorString = FString("");
					if (CaseObject->TryGetStringField("error", ErrorString))
					{
						if (!(hasError && ErrorString == "parse")) {
							UE_LOG(LogArcwarePlugin, Log, TEXT("DIDNT FIND EXPECTED ERROR"));
						}
					}
				}

				// Compare the result (Only applicable in conditions)
				if (CaseObject->HasField("result"))
				{
					std::any result = output.result;
					std::string errors = CompareResults(CaseObject, result);
					if (errors.size() > 0) {
						UE_LOG(LogArcwarePlugin, Error, TEXT("Errors in result:\n%s"), *FString(code.c_str()));
						UE_LOG(LogArcwarePlugin, Error, TEXT("%s"), *FString(errors.c_str()));
					}
				}

				// Compare the variable changes
				if (CaseObject->HasField("result"))
				{
					TSharedPtr<FJsonObject> expectedVarsJson = CaseObject->GetObjectField("changes");
					std::map<std::string, std::any> expectedVars = GetExpectedVars(expectedVarsJson);
					std::string errors = CompareVars(expectedVars, output.changes);
					if (errors.size() > 0) {
						UE_LOG(LogArcwarePlugin, Error, TEXT("Errors in code:\n%s"), UTF8_TO_TCHAR(code.c_str()));
						UE_LOG(LogArcwarePlugin, Error, TEXT("%s"), UTF8_TO_TCHAR(errors.c_str()));
					}
				}

				// Compare the outputs
				if (CaseObject->HasField("output"))
				{
					std::string expectedOutput = TCHAR_TO_UTF8(*CaseObject->GetStringField("output"));
					if (output.output != expectedOutput) {
						UE_LOG(LogArcwarePlugin, Error, TEXT("Errors in code:\n%s"), UTF8_TO_TCHAR(code.c_str()));
						UE_LOG(LogArcwarePlugin, Error, TEXT("Expected output different from actual:"));
						UE_LOG(LogArcwarePlugin, Error, TEXT("Expected: \"%s\"\nActual: \"%s\""), UTF8_TO_TCHAR(expectedOutput.c_str()), UTF8_TO_TCHAR(output.output.c_str()));
					}
				}
			}
		}
		else
		{
			UE_LOG(LogArcwarePlugin, Error, TEXT("Error transpiling arcscript, missing cases field"));
		}
	}

	UE_LOG(LogArcwarePlugin, Warning, TEXT("Test failed!"));
	return false;
}

void FArcweaveModule::TraspileArcscript()
{
	// Initialize the transpiler (for now ignore the element ID)
	//ArcscriptTranspiler transpiler(currentElement, initialVars, currentVisits);
	// Run the script in the Transpiler
	//TranspilerOutput output;
}

/**
 * Creates the Variable instances for the variables from the imported JSON.
 * @param initialVarsJson The json object of the initial variable values.
 * @returns A map with the variable ID as key and the Variable instance as value.
*/
std::map<std::string, Variable> FArcweaveModule::GetInitialVars(TSharedPtr<FJsonObject> JsonObject) {
	const TSharedPtr<FJsonObject>* InitialVarsObject;
	std::map<std::string, Variable> initialVars;
	
	if (JsonObject->TryGetObjectField("initialVars", InitialVarsObject))
	{
		for (const auto& VarObj : (*InitialVarsObject)->Values)
		{
			Variable var;
			TSharedPtr<FJsonObject> VarObject = VarObj.Value->AsObject();
			if(VarObject.IsValid() && VarObject->HasField("type"))
			{
				var.id =  TCHAR_TO_UTF8(*VarObject->GetStringField("id"));
				var.name =  TCHAR_TO_UTF8(*VarObject->GetStringField("name"));
				var.type =  TCHAR_TO_UTF8(*VarObject->GetStringField("type"));
				
				if (var.type == "string") {
					var.value = VarObject->GetStringField("value");
				}
				else if (var.type == "integer") {
					var.value = VarObject->GetIntegerField("value");
				}
				else if (var.type == "boolean") {
					var.value = VarObject->GetBoolField("value");
				}
				else if (var.type == "float") {
					var.value = VarObject->GetNumberField("value");
				}
				initialVars[var.id] = var;
			}
			else
			{
				UE_LOG(LogArcwarePlugin, Error, TEXT("Error reading intial vars, missing type field"));
			}
		}
	}
	return initialVars;
}

std::string FArcweaveModule::CompareResults(TSharedPtr<FJsonObject> expected, std::any actual) {
  // std::cout << "Expected: "<< expected << std::endl;
  std::ostringstream errors;
	if (!actual.has_value()) {
		errors << "Expected result != Actual result:" << std::endl;
		errors << "Expected: ";

		// Serialize the JSON object to an FString
		FString ExpectedJsonString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&ExpectedJsonString);
		FJsonSerializer::Serialize(expected.ToSharedRef(), JsonWriter);
        
		// Convert the FString to std::string and add it to the error message
		errors << TCHAR_TO_UTF8(*ExpectedJsonString) << std::endl;
		errors << "Actual: No Value!" << std::endl;
	}else if (actual.type() == typeid(std::string)) {
    std::string actualStr = std::any_cast<std::string>(actual);
    std::string expectedStr = TCHAR_TO_UTF8(*expected->GetStringField("result"));

    // std::cout << "Actual: "<< actualStr << std::endl;
    if (actualStr != expectedStr) {
      errors << "Expected result != Actual result:" << std::endl;
      errors << "Expected: " << expectedStr << std::endl;
      errors << "Actual: " << actualStr << std::endl;
    }
  }
  else if (actual.type() == typeid(int)) {
    int actualInt = std::any_cast<int>(actual);
    int expectedInt = expected->GetIntegerField("result");

    // std::cout << "Actual: "<< actualInt << std::endl;
    if (actualInt != expectedInt) {
      errors << "Expected result != Actual result:" << std::endl;
      errors << "Expected: " << expectedInt << std::endl;
      errors << "Actual: " << actualInt << std::endl;
    }
  }
  else if (actual.type() == typeid(double)) {
    double actualDbl = std::any_cast<double>(actual);
    double expectedDbl = expected->GetNumberField("result");

    // std::cout << "Actual: "<< actualDbl << std::endl;
    if (actualDbl != expectedDbl) {
      errors << "Expected result != Actual result:" << std::endl;
      errors << "Expected: " << expectedDbl << std::endl;
      errors << "Actual: " << actualDbl << std::endl;
    }
  }
  else if (actual.type() == typeid(bool)) {
    bool actualBool = std::any_cast<bool>(actual);
    bool expectedBool = expected->GetBoolField("result");

    // std::cout << "Actual: "<< actualBool << std::endl;
    if (actualBool != expectedBool) {
      errors << "Expected result != Actual result:" << std::endl;
      errors << "Expected: " << expectedBool << std::endl;
      errors << "Actual: " << actualBool << std::endl;
    }
  } else {
    errors << "Actual type unknown: " << actual.type().name() << std::endl;
  }

  return errors.str();
}

/**
 * Creates the map with the expected variables from a certain test
*/
std::map<std::string, std::any> FArcweaveModule::GetExpectedVars(TSharedPtr<FJsonObject> expectedVarsJson) {
	std::map<std::string, std::any> expectedVars;
	TMap<FString, TSharedPtr<FJsonValue>> expectedVarsJsonMap = expectedVarsJson->Values;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& varPair : expectedVarsJsonMap)
	{
		FString key = varPair.Key;
		TSharedPtr<FJsonValue> jsonValue = varPair.Value;

		if (jsonValue->Type == EJson::String)
		{
			FString value = jsonValue->AsString();
			expectedVars[std::string(TCHAR_TO_UTF8(*key))] = std::string(TCHAR_TO_UTF8(*value));
		}
		else if (jsonValue->Type == EJson::Boolean)
		{
			bool value = jsonValue->AsBool();
			expectedVars[std::string(TCHAR_TO_UTF8(*key))] = value;
		}
		else if (jsonValue->Type == EJson::Number)
		{
			double dblValue = jsonValue->AsNumber();
			if (floor(dblValue) == dblValue)
			{
				int intValue = static_cast<int>(dblValue);
				expectedVars[std::string(TCHAR_TO_UTF8(*key))] = intValue;
			}
			else
			{
				expectedVars[std::string(TCHAR_TO_UTF8(*key))] = dblValue;
			}
		}
	}
	return expectedVars;
}

/**
 * Compares two maps of variables for their values and types. It ignores the int/double distinction.
 * @param expected The expected variable values
 * @param actual The actual variable values
 * @returns A string containing the output messages of the comparison.
*/
std::string FArcweaveModule::CompareVars(std::map<std::string, std::any> expected, std::map<std::string, std::any> actual) {
  std::map<std::string, std::any>::iterator mIt = expected.begin();
  std::ostringstream errors;
  bool hasErrors = false;
  while (mIt != expected.end()) {
    std::string varId = mIt->first;
    std::any expValue = mIt->second;
    std::any actValue = actual[varId];
    if (expValue.type() == typeid(int)) {
      int exp = std::any_cast<int>(expValue);
      if (actValue.type() == typeid(int)) {
        int act = std::any_cast<int>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      } else if (actValue.type() == typeid(double)) {
        double act = std::any_cast<double>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      } else {
        hasErrors = true;
        errors << "Error in test: " << std::endl;
        errors << "Wrong value types. Expected: " << expValue.type().name() << ", actual: " << actValue.type().name() << std::endl;
      }
    } else if (expValue.type() == typeid(double)) {
      double exp = std::any_cast<double>(expValue);
      if (actValue.type() == typeid(int)) {
        int act = std::any_cast<int>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      } else if (actValue.type() == typeid(double)) {
        double act = std::any_cast<double>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      } else {
        hasErrors = true;
        errors << "Error in test: " << std::endl;
        errors << "Wrong value types. Expected: " << expValue.type().name() << ", actual: " << actValue.type().name() << std::endl;
      }
    } else if (expValue.type() == typeid(bool)) {
      if (actValue.type() != typeid(bool)) {
        hasErrors = true;
        errors << "Error in test: " << std::endl;
        errors << "Wrong value types. Expected: " << expValue.type().name() << ", actual: " << actValue.type().name() << std::endl;
      } else {
        bool exp = std::any_cast<bool>(expValue);
        bool act = std::any_cast<bool>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      }
    } else if (expValue.type() == typeid(std::string)) {
      if (actValue.type() != typeid(std::string)) {
        hasErrors = true;
        errors << "Error in test: " << std::endl;
        errors << "Wrong value types. Expected: " << expValue.type().name() << ", actual: " << actValue.type().name() << std::endl;
      } else {
        std::string exp = std::any_cast<std::string>(expValue);
        std::string act = std::any_cast<std::string>(actValue);
        if (exp != act) {
          hasErrors = true;
          errors << "Error in test: " << std::endl << "var " << varId << ": " << std::endl << "Expected: " << exp << "\nActual: " << act << std::endl;
        }
      }
    }

    mIt++;
  }
  return errors.str();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FArcweaveModule, arcweave)
