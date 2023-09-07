// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ArcweaveTypes.generated.h"

/**
 * Types for arweave to unreal engine
 */

//struct for dialogue
USTRUCT(BlueprintType)
struct FArcDialogue
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	FString Name = FString("");

	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	FString Content = FString("");

	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	TArray<FString> Choices;

	FArcDialogue()
		: Name(FString(""))
		, Content(FString(""))
		, Choices(TArray<FString>())
	{}
};

USTRUCT(BlueprintType)
struct FArcweaveVariable
{
	GENERATED_BODY()
	
	FString Id;
	FString Name;
	FString Type;
	TSharedPtr<FJsonValue> Value; // Can hold different types, but you might need to handle serialization/deserialization.

	FArcweaveVariable() {
		Id = FString("");
		Name = FString("");
		Type = FString("");
		Value = nullptr;
	}
};

enum FArcscriptInputType {
	CONDITION,
	SCRIPT
};

struct FArcscriptVariableChange
{
	FString Id;
	FString Type;
	TSharedPtr<FJsonValue> Value;

	FArcscriptVariableChange() {
		Id = FString("");
		Type = FString("");
		Value = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FArcscriptTranspilerOutput
{
	GENERATED_BODY()

	FString Output;
	FArcscriptInputType Type;
	TArray<FArcscriptVariableChange> Changes;
	bool ConditionResult;

	FArcscriptTranspilerOutput() {
		Output = FString("");
		Type = FArcscriptInputType::SCRIPT;
		Changes = TArray<FArcscriptVariableChange>();
		ConditionResult = false;
	}
};