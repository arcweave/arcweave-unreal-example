// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArcweaveSubsytem.generated.h"

//struct for dialogue
USTRUCT(BlueprintType)
struct FArcweaveDialogue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	FString Name = FString("");

	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	FString Content = FString("");

	UPROPERTY(BlueprintReadWrite, Category = "Arcweave")
	TArray<FString> Choices;

	FArcweaveDialogue()
	{
		Name = FString("");
		Content = FString("");
		Choices = TArray<FString>();
	}
};

/**
 * 
 */
UCLASS()
class ARCWEAVE_API UArcweaveSubsytem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Arcweave")
	void FetchData(){};
};
