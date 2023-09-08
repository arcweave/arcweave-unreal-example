// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ArcweaveTypes.h"
#include "UArcscriptTranspilerWrapper.generated.h"

/**
 * 
 */
UCLASS()
class ARCWEAVE_API UArcscriptTranspilerWrapper : public UObject
{
	GENERATED_BODY()
public:

private:

    /** The DLL type of the script input type */
	enum InputType {
		CONDITION,
		SCRIPT
	};

    /** The DLL type of the variable changes after a script run */
	struct VariableChange {
		char* varId;
		const char* type;
		int int_result;
		double double_result;
		char* string_result;
		bool bool_result;
		VariableChange() {
			varId = nullptr;
			type = nullptr;
			int_result = 0;
			double_result = 0.0;
			string_result = nullptr;
			bool_result = false;
		}
	};

    /** The DLL type of the script output */
	struct TranspilerOutput {
		char* output;
		InputType type;
		VariableChange* changes;
		size_t changesLen;
		bool conditionResult;

		TranspilerOutput() {
			output = nullptr;
			type = InputType::SCRIPT;
			changes = nullptr;
			changesLen = 0;
			conditionResult = false;
		}
	};

    /** The DLL type of the initial input variables of a script */
	struct Variable {
		const char* id;
		const char* name;
		const char* type;
		int int_val;
		double double_val;
		const char* string_val;
		bool bool_val;
		Variable() {
			id = nullptr;
			name = nullptr;
			type = nullptr;
			int_val = 0;
			double_val = 0.0;
			string_val = nullptr;
			bool_val = false;
		}
	};

    /** The DLL type of the initial visits of a script */
	struct Visit {
		const char* elId;
		int visits;
		Visit() {
			elId = nullptr;
			visits = 0;
		}
	};

public:
	typedef TranspilerOutput(__cdecl* __RunScript)(const char*, const char*, Variable*, size_t, Visit*, size_t);
	__RunScript m_runScript;

	UArcscriptTranspilerWrapper() {
		m_runScript = nullptr;
	};

    /** Run Arcscript code using the external library DLL */
	FArcscriptTranspilerOutput RunScript(FString code, FString elementId, TMap<FString, FArcweaveVariable> initialVars, TMap<FString, int> visits);
	// Additional helper methods can be added here if needed.
};	

