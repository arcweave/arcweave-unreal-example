// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArcweaveSubsystem.generated.h"

struct FArcweaveAPISettings;

USTRUCT

(BlueprintType)
struct FArcweaveBoard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Arcweave")
	FString BoardId; // This will hold the key of each board in the JSON map

	UPROPERTY(BlueprintReadOnly, Category = "Arcweave")
	FString Name;
};

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArcweaveResponseReceived, const TArray<FArcweaveBoard>&, Boards);

UCLASS()
class ARCWEAVE_API UArcweaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/*
	 * Fetch the data from Arcweave API
	 */
	UFUNCTION(BlueprintCallable, Category = "Arcweave")
	void FetchData(FString APIToken, FString ProjectHash);
	/*
	 * Get Arcweave API token from settings
	 */
	UFUNCTION(BlueprintPure, Category = "Arcweave")
	FArcweaveAPISettings GetArcweaveSettings() const;

	UPROPERTY(BlueprintAssignable, Category = "Arcweave")
	FOnArcweaveResponseReceived OnArcweaveResponseReceived;
private:
	void HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

