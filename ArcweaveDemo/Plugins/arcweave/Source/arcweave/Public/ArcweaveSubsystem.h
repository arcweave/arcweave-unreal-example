// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArcweaveSubsystem.generated.h"

USTRUCT(BlueprintType)
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
	UFUNCTION(BlueprintCallable, Category = "Arcweave")
	void FetchData();
	UPROPERTY(BlueprintAssignable, Category = "Arcweave")
	FOnArcweaveResponseReceived OnArcweaveResponseReceived;
private:
	void HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

