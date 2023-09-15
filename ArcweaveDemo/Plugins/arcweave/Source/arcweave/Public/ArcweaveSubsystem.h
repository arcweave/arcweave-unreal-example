// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArcweaveTypes.h"
#include "ArcweaveSubsystem.generated.h"

struct FArcweaveAPISettings;
class UArcscriptTranspilerWrapper;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArcweaveResponseReceived, const FArcweaveProjectData&, ArcweaveProjectData);

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
    FArcweaveAPISettings LoadArcweaveSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Arcweave")
    void SaveArcweaveSettings(const FString& APIToken, const FString& ProjectHash);

    UFUNCTION(BlueprintPure, Category = "Arcweave")
    FArcweaveProjectData GetArcweaveProjectData() const {return ProjectData;};

	UPROPERTY(BlueprintAssignable, Category = "Arcweave")
	FOnArcweaveResponseReceived OnArcweaveResponseReceived;

protected:
    //override init function
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
private:
	void HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    FString RemoveHtmlTags(const FString& InputString);
    TArray<FArcweaveAssetData> ParseComponentAsset(const TSharedPtr<FJsonObject>& ComponentValueObject);
    TArray<FArcweaveAttributeData> ParseComponentAttributes(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& ComponentValueObject);
    void ParseAttributeValue(const TSharedPtr<FJsonObject>& ValueObject, FArcweaveAttributeValueData& AttributeValue);
    TArray<FArcweaveBoardData> ParseBoard(const TSharedPtr<FJsonObject>& MainJsonObject);
    TMap<FString, FArcweaveVariable> ParseVariables(const TSharedPtr<FJsonObject>& MainJsonObject);
    TArray<FArcweaveConnectionsData> ParseConnections(const FString& FieldName, const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& BoardValueObject);
    TArray<FArcweaveElementData> ParseElements(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& BoardValueObject);
    TArray<FArcweaveComponentData> ParseComponents(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& ElementValueObject);
    TArray<FArcweaveComponentData> ParseAllComponents(const TSharedPtr<FJsonObject>& MainJsonObject);
    void ParseResponse(const FString& ResponseString);
    FArcscriptTranspilerOutput RunTranspiler(FString Code, FString ElementId, TMap<FString, FArcweaveVariable> InitialVars, TMap<FString, int> Visits);

    //void LogStructFields(const void* StructPtr, UStruct* StructDefinition);
    //void LogStructFieldsRecursive(const void* StructPtr, UStruct* StructDefinition, int32 IndentationLevel);

private:
    UPROPERTY()
    UArcscriptTranspilerWrapper* ArcscriptWrapper;
    
    FArcweaveProjectData ProjectData = FArcweaveProjectData();
};

