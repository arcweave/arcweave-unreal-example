﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "UArcscriptTranspilerWrapper.h"

#include "Core.h"
#include <string>

FArcscriptTranspilerOutput UArcscriptTranspilerWrapper::RunScript(FString code, FString elementId, TMap<FString, FArcweaveVariable> initialVars, TMap<FString, int> visits) {
	int varLength = initialVars.Num();
	int visitsLength = visits.Num();
	const char* dllCode = strdup(TCHAR_TO_UTF8(*code));
	const char* dllElId = strdup(TCHAR_TO_UTF8(*elementId));
	Variable* dllVars = new Variable[varLength];
	Visit* dllVisits = new Visit[visitsLength];

	int i = 0;
	for (auto& var : initialVars) {
		dllVars[i].id = strdup(TCHAR_TO_UTF8(*var.Value.Id));
		dllVars[i].name = strdup(TCHAR_TO_UTF8(*var.Value.Name));

		if (var.Value.Type.Equals(TEXT("string"))) {
			dllVars[i].type = "string";
			dllVars[i].string_val = strdup(TCHAR_TO_UTF8(*(var.Value.Value.Get()->AsString())));
			UE_LOG(LogTemp, Warning, TEXT("%d: ID: %s, Name: %s, Type: %s, Value: %s"), i, *FString(dllVars[i].id), *FString(dllVars[i].name), *FString(dllVars[i].type), *FString(dllVars[i].string_val));
		}
		else if (var.Value.Type.Equals(TEXT("integer"))) {
			dllVars[i].type = "integer";
			dllVars[i].int_val = var.Value.Value.Get()->AsNumber();
			UE_LOG(LogTemp, Warning, TEXT("%d: ID: %s, Name: %s, Type: %s, Value: %d"), i, *FString(dllVars[i].id), *FString(dllVars[i].name), *FString(dllVars[i].type), dllVars[i].int_val);
		}
		else if (var.Value.Type.Equals(TEXT("double"))) {
			dllVars[i].type = "double";
			dllVars[i].double_val = var.Value.Value.Get()->AsNumber();
			UE_LOG(LogTemp, Warning, TEXT("%d: ID: %s, Name: %s, Type: %s, Value: %f"), i, *FString(dllVars[i].id), *FString(dllVars[i].name), *FString(dllVars[i].type), dllVars[i].double_val);
		}
		else if (var.Value.Type.Equals(TEXT("boolean"))) {
			dllVars[i].type = "bool";
			dllVars[i].bool_val = var.Value.Value.Get()->AsBool();
			UE_LOG(LogTemp, Warning, TEXT("%d: ID: %s, Name: %s, Type: %s, Value: %d"), i, *FString(dllVars[i].id), *FString(dllVars[i].name), *FString(dllVars[i].type), dllVars[i].bool_val);
		}
		i++;
	}
	for (i = 0; i < varLength; i++) {
		FString varOutput = FString::Printf(TEXT("%d: Var ID: %s, Name: %s, Type: %s, Value: "), i, *FString(dllVars[i].id), *FString(dllVars[i].name), *FString(dllVars[i].type));
		if (strcmp(dllVars[i].type, "string") == 0) {
			varOutput += FString(dllVars[i].string_val);
		}
		else if (strcmp(dllVars[i].type, "integer") == 0) {
			varOutput += FString::FromInt(dllVars[i].int_val);
		}
		else if (strcmp(dllVars[i].type, "double") == 0) {
			varOutput += FString::SanitizeFloat(dllVars[i].double_val);
		}
		else if (strcmp(dllVars[i].type, "bool") == 0) {
			varOutput += FString::FromInt(dllVars[i].bool_val);
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *varOutput);
	}
	i = 0;
	for (auto& visit : visits) {
		dllVisits[i].elId = strdup(TCHAR_TO_UTF8(*visit.Key));
		dllVisits[i].visits = visit.Value;
		i++;
	}

	TranspilerOutput dllResult = m_runScript(dllCode, dllElId, dllVars, varLength, dllVisits, visitsLength);

	FArcscriptTranspilerOutput result;
	result.Output = dllResult.output;
	result.ConditionResult = dllResult.conditionResult;
	if (dllResult.type == InputType::CONDITION) {
		result.Type = FArcscriptInputType::CONDITION;
	}
	else {
		result.Type = FArcscriptInputType::SCRIPT;
	}

	for (i = 0; i < dllResult.changesLen; i++) {
		FArcscriptVariableChange change;
		change.Id = FString(dllResult.changes[i].varId);
		change.Type = FString(dllResult.changes[i].type);
		if (strcmp(dllResult.changes[i].type, "string") == 0) {
			change.Value = MakeShareable(new FJsonValueString(dllResult.changes[i].string_result));
		}
		else if (strcmp(dllResult.changes[i].type, "integer") == 0) {
			change.Value = MakeShareable(new FJsonValueNumber(dllResult.changes[i].int_result));
		}
		else if (strcmp(dllResult.changes[i].type, "double") == 0) {
			change.Value = MakeShareable(new FJsonValueNumber(dllResult.changes[i].double_result));
		}
		else if (strcmp(dllResult.changes[i].type, "bool") == 0) {
			change.Value = MakeShareable(new FJsonValueBoolean(dllResult.changes[i].bool_result));
		}
		result.Changes.Add(change);
	}

	FString lines;
	for (i = 0; i < dllResult.changesLen; i++) {
		FString line = dllResult.changes[i].varId + FString(": ");
		if (strcmp(dllResult.changes[i].type, "string") == 0) {
			line += dllResult.changes[i].string_result;
		}
		else if (strcmp(dllResult.changes[i].type, "integer") == 0) {
			line += FString::FromInt(dllResult.changes[i].int_result);
		}
		else if (strcmp(dllResult.changes[i].type, "double") == 0) {
			line += FString::SanitizeFloat(dllResult.changes[i].double_result);
		}
		else if (strcmp(dllResult.changes[i].type, "bool") == 0) {
			line += dllResult.changes[i].bool_result ? FString("true") : FString("false");
		}
		line += FString("\n");

		lines += line;
	}
	//FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(lines));


	return result;
}