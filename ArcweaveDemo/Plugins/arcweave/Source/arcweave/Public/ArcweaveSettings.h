// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcweaveSettings.generated.h"



/**
 * 
 */
UCLASS(config = Game, DefaultConfig)
class ARCWEAVE_API UArcweaveSettings : public UObject
{
	GENERATED_BODY()

public:
	UArcweaveSettings();

	/*
	 * API token that you can find in your Arcweave account settings.
	 */
	UPROPERTY(Config, EditAnywhere, Category = ArcweaveSettings)
	FString APIToken = FString("");

	/*
	 * Project hash that we want to retrieve the information for. You can find it by looking at the URL of your project.
	 */
	UPROPERTY(Config, EditAnywhere, Category = ArcweaveSettings)
	FString Hash = FString("");

	//override post init properties to check if the settings are valid
	virtual void PostInitProperties() override;
};

// vsvIOEPSAorYs8qTlPvsHeKQ4MksRyAVOC6m09DB1xwgqEaMpV3ppmLnCNOs
// https://arcweave.com/app/project/omE79ga0RN?board=1c11fa2a-f76c-4be7-aac4-6bf92a63240f&scale=0.909091&coords=-10223.8,-10206.8
// omE79ga0RN
