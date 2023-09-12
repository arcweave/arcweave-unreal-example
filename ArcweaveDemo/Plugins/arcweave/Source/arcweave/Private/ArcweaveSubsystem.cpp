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
	    FString ResponseString = Response->GetContentAsString();
	    UE_LOG(LogTemp, Log, TEXT("HTTP Response: %s"), *ResponseString);

	    // Convert the response to a JSON object
	    TSharedPtr<FJsonObject> JsonObject;
	    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	    {
	        // Failed to parse the response
	        UE_LOG(LogTemp, Error, TEXT("Failed to parse the HTTP Response!"));
	        return;
	    }

	    // Extract project name and cover data
	    FArcweaveProjectData ProjectData = FArcweaveProjectData();
	    if (JsonObject->TryGetStringField("name", ProjectData.Name))
	    {
	        if (JsonObject->HasField("cover"))
	        {
	            TSharedPtr<FJsonObject> CoverObject = JsonObject->GetObjectField("cover");

	            // Extract cover data
	            if (CoverObject->TryGetStringField("file", ProjectData.Cover.File) &&
                    CoverObject->TryGetStringField("type", ProjectData.Cover.Type))
	            {
	                // Successfully extracted cover data
	            }
	        }

	        // Extract boards
	        const TSharedPtr<FJsonObject>* BoardsObject;
	        if (JsonObject->TryGetObjectField("boards", BoardsObject))
	        {
	            for (const auto& BoardPair : BoardsObject->Get()->Values)
	            {
	                FArcweaveBoardData Board;
	                Board.BoardId = BoardPair.Key;

	                const TSharedPtr<FJsonObject> BoardValueObject = BoardPair.Value->AsObject();
	                if (BoardValueObject.IsValid())
	                {
	                    FString BoardName;
	                    if (BoardValueObject->TryGetStringField("name", BoardName))
	                    {
	                        Board.Name = BoardName;
	                    }

	                    TArray<FString> ElementArrayStrings;
	                    if (BoardValueObject->TryGetStringArrayField("elements", ElementArrayStrings))
	                    {
                            for (const auto& ElementId : ElementArrayStrings)
                            {
                               // Extract elements
	                            const TSharedPtr<FJsonObject>* ElementsObject;
	                            if (JsonObject->TryGetObjectField("elements", ElementsObject))
	                            {
	                                for (const auto& ElementPair : ElementsObject->Get()->Values)
	                                {
	                                    FArcweaveElementData Element;
	                                    Element.Id = ElementPair.Key;

                                        if (ElementPair.Key == ElementId)
                                        {
	                                        const TSharedPtr<FJsonObject> ElementValueObject = ElementPair.Value->AsObject();
	                                        if (ElementValueObject.IsValid())
	                                        {
	                                            FString ElementTheme;
	                                            FString ElementTitle;
	                                            FString ElementContent;
	                                            if (ElementValueObject->TryGetStringField("theme", ElementTheme) &&
                                                    ElementValueObject->TryGetStringField("title", ElementTitle) &&
                                                    ElementValueObject->TryGetStringField("content", ElementContent))
	                                            {
	                                                Element.Theme = ElementTheme;
	                                                Element.Title = ElementTitle;
	                                                Element.Content = ElementContent;

	                                                // Extract "outputs" as an array of strings
	                                                TArray<FString> Outputs;
	                                                if (ElementValueObject->TryGetStringArrayField("outputs", Outputs))
	                                                {
	                                                    Element.Outputs = Outputs;
	                                                }

	                                                // Extract "components" as an array of component data structs (similar to "boards")
	                                                FArcweaveComponentData ElComponent;
	                                                TArray<FString> ComponentsStringArray;
                                                    if (ElementValueObject->TryGetStringArrayField("components", ComponentsStringArray))
                                                    {
                                                        for (const auto& ComponentId : ComponentsStringArray)
                                                        {
                                                            //iterrare thirugh the array of atrributes and find with this id
                                                            const TSharedPtr<FJsonObject>* CompObject;
                                                            if (JsonObject->TryGetObjectField("components", CompObject))
                                                            {
                                                                for (const auto& CompPair : CompObject->Get()->Values)
                                                                {
                                                                    ElComponent.Id = CompPair.Key;

                                                                    if (CompPair.Key == ComponentId)
                                                                    {
                                                                        const TSharedPtr<FJsonObject> ComponentValueObject = CompPair.Value->AsObject();
                                                                        if (ComponentValueObject.IsValid())
                                                                        {
                                                                            // Extract the "name"
                                                                            FString ComponentName;
                                                                            bool ComponentRoot;
                                                                            if (ComponentValueObject->TryGetStringField("name", ComponentName))
                                                                            {
                                                                                ElComponent.Name = ComponentName;
                                                                            }

                                                                            //Exttract the root
                                                                            if (ComponentValueObject->TryGetBoolField("root", ComponentRoot))
                                                                            {
                                                                                ElComponent.Root = ComponentRoot;
                                                                            }

                                                                            // Extract "children" as an array of strings
                                                                            TArray<FString> Children;
                                                                            if (ComponentValueObject->TryGetStringArrayField("children", Children))
                                                                            {
                                                                                ElComponent.Children = Children;
                                                                            }

                                                                            // Extract "assets" for components as a map of asset data structs
                                                                            const TSharedPtr<FJsonObject>* ComponentAssetsObject;
                                                                            if (ComponentValueObject->TryGetObjectField("assets", ComponentAssetsObject))
                                                                            {
                                                                                FArcweaveAssetData ComponentAsset;
                                                                                for (const auto& ComponentAssetPair : ComponentAssetsObject->Get()->Values)
                                                                                {
                                                                                    ComponentAsset.AudioId = ComponentAssetPair.Key;

                                                                                    const TSharedPtr<FJsonObject> ComponentAssetValueObject = ComponentAssetPair.Value->AsObject();
                                                                                    if (ComponentAssetValueObject.IsValid())
                                                                                    {

                                                                                        // Extract "mode" field for the component asset
                                                                                        FString Mode;
                                                                                        if (ComponentAssetValueObject->TryGetStringField("mode", Mode))
                                                                                        {
                                                                                            ComponentAsset.Mode = Mode;
                                                                                        }

                                                                                        // Extract "asset" field for the component asset
                                                                                        FString Asset;
                                                                                        if (ComponentAssetValueObject->TryGetStringField("asset", Asset))
                                                                                        {
                                                                                            ComponentAsset.Asset = Asset;
                                                                                        }

                                                                                        // Extract "delay" field for the component asset
                                                                                        FString Delay;
                                                                                        if (ComponentAssetValueObject->TryGetStringField("delay", Delay))
                                                                                        {
                                                                                            ComponentAsset.Delay = Delay;
                                                                                        }
                                                                                    }
                                                                                }
                                                                                ElComponent.Assets.Add(ComponentAsset);
                                                                            }

                                                                            // Extract "attributes" for components as a map of asset data structs
                                                                            TArray<FString> AttributesStringArray;
                                                                            if (ComponentValueObject->TryGetStringArrayField("attributes", AttributesStringArray))
                                                                            {
                                                                                for (const auto& AttributeAssetId : AttributesStringArray)
                                                                                {
                                                                                    FArcweaveAttributeData AttributeAsset;

                                                                                    //iterrare thirugh the array of atrributes and find with this id
                                                                                    const TSharedPtr<FJsonObject>* AttributesObject;
                                                                                    if (JsonObject->TryGetObjectField("attributes", AttributesObject))
                                                                                    {
                                                                                        for (const auto& AttrPair : AttributesObject->Get()->Values)
                                                                                        {
                                                                                            FArcweaveAttributeData AttributeData;
                                                                                            AttributeData.Id = AttrPair.Key;

                                                                                            if (AttributeAssetId == AttributeData.Id)
                                                                                            {
                                                                                                const TSharedPtr<FJsonObject> AttributeAssetValueObject = AttrPair.Value->AsObject();
                                                                                                if (AttributeAssetValueObject.IsValid())
                                                                                                {
                                                                                                    FString cId;
                                                                                                    if (AttributeAssetValueObject->TryGetStringField("cId", cId))
                                                                                                    {
                                                                                                        AttributeAsset.cId = cId;
                                                                                                    }

                                                                                                    FString Name;
                                                                                                    if (AttributeAssetValueObject->TryGetStringField("name", Name))
                                                                                                    {
                                                                                                        AttributeAsset.Name = Name;
                                                                                                    }

                                                                                                    FString cType;
                                                                                                    if (AttributeAssetValueObject->TryGetStringField("cType", cType))
                                                                                                    {
                                                                                                        AttributeAsset.cType = cType;
                                                                                                    }

                                                                                                    const TSharedPtr<FJsonObject>* AttributeValueObject;
                                                                                                    if (ComponentValueObject->TryGetObjectField("value", AttributeValueObject))
                                                                                                    {
                                                                                                        for (const auto& AtrributeValuePair : AttributeValueObject->Get()->Values)
                                                                                                        {
                                                                                                            FArcweaveAttributeValueData ArcweaveAttributeValueData;

                                                                                                            const TSharedPtr<FJsonObject> AttributeValueObj = AtrributeValuePair.Value->AsObject();
                                                                                                            if (AttributeValueObj.IsValid())
                                                                                                            {
                                                                                                                // Extract the "name"
                                                                                                                FString Data;
                                                                                                                FString Type;
                                                                                                                bool Plain;
                                                                                                                if (AttributeValueObj->TryGetStringField("data", Data))
                                                                                                                {
                                                                                                                    ArcweaveAttributeValueData.Data = Data;
                                                                                                                }
                                                                                                                if (AttributeValueObj->TryGetStringField("type", Type))
                                                                                                                {
                                                                                                                    ArcweaveAttributeValueData.Type = Type;
                                                                                                                }
                                                                                                                if (AttributeValueObj->TryGetBoolField("plain", Plain))
                                                                                                                {
                                                                                                                    ArcweaveAttributeValueData.Plain = Plain;
                                                                                                                }

                                                                                                                AttributeAsset.Value = ArcweaveAttributeValueData;
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                                ElComponent.Attributes.Add(AttributeAsset);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                            // Add the fully populated Component struct to the Element's Components array
                                                                            Element.Components.Add(ElComponent);
                                                                        }
                                                                    Element.Components.Add(ElComponent);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
	                                            }
	                                        }
                                            Board.Elements.Add(Element);
                                        }
	                                }
	                            }
                            }
	                    }

	                    TArray<FString> ConnectionsArrayStrings;
	                    if (BoardValueObject->TryGetStringArrayField("connections", ConnectionsArrayStrings))
	                    {
	                        for (const auto& ConnectionId : ConnectionsArrayStrings)
	                        {
	                            // Extract connections
	                            const TSharedPtr<FJsonObject>* ConnectionsObject;
	                            if (JsonObject->TryGetObjectField("connections", ConnectionsObject))
	                            {
	                                for (const auto& ConnectionPair : ConnectionsObject->Get()->Values)
	                                {
	                                    FArcweaveConnectionsData Connection;
	                                    Connection.Id = ConnectionPair.Key;

	                                    if (ConnectionPair.Key == ConnectionId)
	                                    {
	                                        const TSharedPtr<FJsonObject> ConObject = ConnectionPair.Value->AsObject();
	                                        if (ConObject.IsValid())
	                                        {
	                                            FString Type;
	                                            if (ConObject->TryGetStringField("type", Type))
	                                            {
	                                                Connection.Type = Type;
	                                            }

	                                            FString Label;
	                                            if (ConObject->TryGetStringField("label", Label))
	                                            {
	                                                Connection.Label = Label;
	                                            }

	                                            FString Theme;
	                                            if (ConObject->TryGetStringField("theme", Theme))
	                                            {
	                                                Connection.Theme = Theme;
	                                            }

	                                            FString Sourceid;
	                                            if (ConObject->TryGetStringField("sourceid", Sourceid))
	                                            {
	                                                Connection.Sourceid = Sourceid;
	                                            }

	                                            FString Targetid;
	                                            if (ConObject->TryGetStringField("theme", Targetid))
	                                            {
	                                                Connection.Targetid = Targetid;
	                                            }

	                                            FString SourceType;
	                                            if (ConObject->TryGetStringField("sourceType", SourceType))
	                                            {
	                                                Connection.SourceType = SourceType;
	                                            }

	                                            FString TargetType;
	                                            if (ConObject->TryGetStringField("targetType", TargetType))
	                                            {
	                                                Connection.TargetType = TargetType;
	                                            }
	                                        }
	                                        Board.Connections.Add(Connection);
	                                    }
	                                }
	                                
	                            }
	                        }
	                    }
	                }
	            }
	        }
	        // Fire the multicast delegate
	        OnArcweaveResponseReceived.Broadcast(ProjectData);
	    }
	    else
	    {
	        // Handle error here.
	        UE_LOG(LogTemp, Error, TEXT("Project name is invalid!"));
	    }
	}
	else
	{
		// Handle error here.
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
	}
}

