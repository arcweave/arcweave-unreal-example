// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\ArcweaveSubsystem.h"

#include "ArcweaveSettings.h"
#include "ArcweaveTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UArcweaveSubsystem::FetchData(FString APIToken, FString ProjectHash)
{
	/*UArcweaveSettings* ArcweaveSettings = GetMutableDefault<UArcweaveSettings>();
	// Check if the settings are valid
	if (ArcweaveSettings->APIToken.IsEmpty() || ArcweaveSettings->Hash.IsEmpty())
	{
		//log also to screen
		UE_LOG(LogTemp, Error, TEXT("Arcweave settings are not valid!"));
		if (GEngine)
		{
			const float TimeToDisplay = 5.0f; 
			const FColor TextColor = FColor::Red; 
			const FString Message = TEXT("Arcweave settings are not valid!"); 

			GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, TextColor, Message);
		}
		return;
	}*/
	FString ApiUrl = FString::Printf(TEXT("https://arcweave.com/api/%s/json"), *ProjectHash);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL(ApiUrl);
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *APIToken));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	// Set the request complete callback
	Request->OnProcessRequestComplete().BindUObject(this, &UArcweaveSubsystem::HandleFetch);

	// Execute the request
	Request->ProcessRequest();
}

FArcweaveAPISettings UArcweaveSubsystem::GetArcweaveSettings() const
{
	FArcweaveAPISettings OutSetttings = FArcweaveAPISettings();	
	UArcweaveSettings* ArcweaveSettings = GetMutableDefault<UArcweaveSettings>();
	// Check if the settings are valid
	if (ArcweaveSettings->APIToken.IsEmpty() || ArcweaveSettings->Hash.IsEmpty())
	{
		//log also to screen
		UE_LOG(LogTemp, Error, TEXT("Arcweave settings are not valid!"));
		if (GEngine)
		{
			const float TimeToDisplay = 5.0f; 
			const FColor TextColor = FColor::Red; 
			const FString Message = TEXT("Arcweave settings are not valid!"); 

			GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, TextColor, Message);
		}
		return OutSetttings;
	}

	OutSetttings.APIToken = ArcweaveSettings->APIToken;
	OutSetttings.Hash = ArcweaveSettings->Hash;
	return OutSetttings;
}

void UArcweaveSubsystem::HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Handle the response content here.
		FString ResponseString = Response->GetContentAsString();
		UE_LOG(LogTemp, Log, TEXT("HTTP Response: %s"), *ResponseString);
		
		// Convert the response to a JSON object
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
		{
			// Failed to parse the response
			UE_LOG(LogTemp, Error, TEXT("Failed to parse the HTTP Response!"));
			return;
		}

		// Extract boards and their names
		TArray<FArcweaveBoard> ReceivedBoards;

		const TSharedPtr<FJsonObject>* BoardsObject;
		if (JsonObject->TryGetObjectField("boards", BoardsObject))
		{
			for (const auto& BoardPair : BoardsObject->Get()->Values)
			{
				FArcweaveBoard Board;
				Board.BoardId = BoardPair.Key;
				if (BoardPair.Value->AsObject()->TryGetStringField("name", Board.Name))
				{
					ReceivedBoards.Add(Board);
				}
			}
		}

		// Fire the multicast delegate
		OnArcweaveResponseReceived.Broadcast(ReceivedBoards);
	}
	else
	{
		// Handle error here.
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
	}
}

