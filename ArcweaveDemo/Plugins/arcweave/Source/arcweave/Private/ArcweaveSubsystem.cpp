// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\ArcweaveSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UArcweaveSubsystem::FetchData()
{
	FString ApiUrl = TEXT("https://arcweave.com/api/POlqqO3laz/json");
	FString AuthToken = TEXT("vsvIOEPSAorYs8qTlPvsHeKQ4MksRyAVOC6m09DB1xwgqEaMpV3ppmLnCNOs");  // replace with your actual token

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL(ApiUrl);
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	// Set the request complete callback
	Request->OnProcessRequestComplete().BindUObject(this, &UArcweaveSubsystem::HandleFetch);

	// Execute the request
	Request->ProcessRequest();
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

// vsvIOEPSAorYs8qTlPvsHeKQ4MksRyAVOC6m09DB1xwgqEaMpV3ppmLnCNOs
// https://arcweave.com/app/project/POlqqO3laz?board=8ee8ac05-de1d-416d-8f3d-8bf0b08594f4&scale=1.1&coords=-10896.5,-9859.31
// POlqqO3laz