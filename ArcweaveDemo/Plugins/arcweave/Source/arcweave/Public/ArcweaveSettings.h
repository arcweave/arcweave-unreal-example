// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcweaveSettings.generated.h"

/**
 * 
 */
UCLASS(config = Engine)
class ARCWEAVE_API UArcweaveSettings : public UObject
{
	GENERATED_BODY()

public:
	UArcweaveSettings();
	
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString APIKey = FString("");
};
