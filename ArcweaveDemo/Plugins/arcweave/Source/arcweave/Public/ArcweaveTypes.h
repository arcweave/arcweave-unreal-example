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

//struct holding all the settings for the plugin
USTRUCT(BlueprintType)
struct FArcweaveAPISettings
{
	GENERATED_BODY()

	/*
	 * API token that you can find in your Arcweave account settings.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FString APIToken = FString("");

	/*
	 * Project hash that we want to retrieve the information for. You can find it by looking at the URL of your project.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FString Hash = FString("");

	FArcweaveAPISettings()
		: APIToken(FString(""))
		, Hash(FString(""))
	{}
};

