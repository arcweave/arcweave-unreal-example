// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\ArcweaveSubsystem.h"

#include "Arcweave.h"
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
		UE_LOG(ArcweaveSubsystem, Error, TEXT("Arcweave settings are not valid!"));
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
		UE_LOG(LogArcwarePlugin, Error, TEXT("Arcweave settings are not valid!"));
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

/*
void UArcweaveSubsystem::HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
	    FString ResponseString = Response->GetContentAsString();
	    UE_LOG(ArcweaveSubsystem, Log, TEXT("HTTP Response: %s"), *ResponseString);

	    // Convert the response to a JSON object
	    TSharedPtr<FJsonObject> JsonObject;
	    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	    {
	        // Failed to parse the response
	        UE_LOG(ArcweaveSubsystem, Error, TEXT("Failed to parse the HTTP Response!"));
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
	                                            ElementValueObject->TryGetStringField("theme", Element.Theme);
	                                            ElementValueObject->TryGetStringField("title", Element.Title);
	                                            ElementValueObject->TryGetStringField("content", Element.Content);
	                                            ElementValueObject->TryGetStringArrayField("outputs", Element.Outputs);

	                                            // Extract "components" as an array of component data structs (similar to "boards")
	                                            FArcweaveComponentData ElComponent;
	                                            TArray<FString> ComponentsStringArray;
	                                            if (ElementValueObject->TryGetStringArrayField("components", ComponentsStringArray))
	                                            {
	                                                for (const auto& ComponentId : ComponentsStringArray)
	                                                {
	                                                    //iterrare thirugh the array of components and find with this id
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
	                                                                    ComponentValueObject->TryGetStringField("name",  ElComponent.Name);
	                                                                    ComponentValueObject->TryGetBoolField("root", ElComponent.Root);
	                                                                    ComponentValueObject->TryGetStringArrayField("children", ElComponent.Children);
                                                                       
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
	                                                                                ComponentAssetValueObject->TryGetStringField("mode", ComponentAsset.Mode);
	                                                                                ComponentAssetValueObject->TryGetStringField("asset", ComponentAsset.Asset);
	                                                                                ComponentAssetValueObject->TryGetStringField("delay", ComponentAsset.Delay);
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
	                                                                                            AttributeAssetValueObject->TryGetStringField("cId", AttributeAsset.cId);
	                                                                                            AttributeAssetValueObject->TryGetStringField("name", AttributeAsset.Name);
	                                                                                            AttributeAssetValueObject->TryGetStringField("cType", AttributeAsset.cType);

	                                                                                            const TSharedPtr<FJsonObject>* AttributeValueObject;
	                                                                                            if (AttributeAssetValueObject->TryGetObjectField("value", AttributeValueObject))
	                                                                                            {
	                                                                                                for (const auto& AtrributeValuePair : AttributeValueObject->Get()->Values)
	                                                                                                {
	                                                                                                    FArcweaveAttributeValueData ArcweaveAttributeValueData;

	                                                                                                    const TSharedPtr<FJsonObject> AttributeValueObj = AtrributeValuePair.Value->AsObject();
	                                                                                                    if (AttributeValueObj.IsValid())
	                                                                                                    {
	                                                                                                        AttributeValueObj->TryGetStringField("data", ArcweaveAttributeValueData.Data);
	                                                                                                        AttributeValueObj->TryGetStringField("type", ArcweaveAttributeValueData.Type);
	                                                                                                        AttributeValueObj->TryGetBoolField("plain", ArcweaveAttributeValueData.Plain);
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
	                ProjectData.Boards.Add(Board);
	            }
	        }
	        // Fire the multicast delegate
	        OnArcweaveResponseReceived.Broadcast(ProjectData);
	    }
	    else
	    {
	        // Handle error here.
	        UE_LOG(ArcweaveSubsystem, Error, TEXT("Project name is invalid!"));
	    }
	}
	else
	{
		// Handle error here.
		UE_LOG(ArcweaveSubsystem, Error, TEXT("HTTP Request failed!"));
	}
}
*/

FString UArcweaveSubsystem::RemoveHtmlTags(const FString& InputString)
{
    FString CleanedString = InputString;

    // Search for the opening '<' character and closing '>' character in the string
    int32 StartIndex = 0;
    while (CleanedString.FindChar('<', StartIndex))
    {
        int32 TagStartIndex = CleanedString.FindChar('<', StartIndex);
        int32 TagEndIndex = CleanedString.FindChar('>', TagStartIndex);

        // If both '<' and '>' are found, remove the tag and update the start index
        if (TagStartIndex != INDEX_NONE && TagEndIndex != INDEX_NONE)
        {
            CleanedString.RemoveAt(TagStartIndex, TagEndIndex - TagStartIndex + 1, /*bAllowShrinking*/ true);
        }
        else
        {
            // If no matching '>' is found, break the loop
            break;
        }

        // Set the start index for the next iteration to continue searching for tags
        StartIndex = TagStartIndex;
    }

    return CleanedString;
}

TArray<FArcweaveAssetData> UArcweaveSubsystem::ParseComponentAsset(const TSharedPtr<FJsonObject>& ComponentValueObject)
{
    // Parse "assets" for components as an array of asset data structs
    TArray<FArcweaveAssetData> ComponentAssets;
    const TSharedPtr<FJsonObject>* ComponentAssetsObject;
    if (ComponentValueObject->TryGetObjectField("assets", ComponentAssetsObject))
    {
        for (const auto& ComponentAssetPair : ComponentAssetsObject->Get()->Values)
        {
            const TSharedPtr<FJsonObject> AssetValueObject = ComponentAssetPair.Value->AsObject();
            FArcweaveAssetData ComponentAsset;
            AssetValueObject->TryGetStringField("mode", ComponentAsset.Mode);
            AssetValueObject->TryGetStringField("asset", ComponentAsset.Asset);
            AssetValueObject->TryGetStringField("delay", ComponentAsset.Delay);
            ComponentAssets.Add(ComponentAsset);
        }
    }
    return  ComponentAssets;
}

TArray<FArcweaveAttributeData> UArcweaveSubsystem::ParseComponentAttributes(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& ComponentValueObject)
{
    TArray<FArcweaveAttributeData> ComponentAttributes;
    TArray<FString> AttributesStringArray;
    if (ComponentValueObject->TryGetStringArrayField("attributes", AttributesStringArray))
    {
        for (const auto& AttributeAssetId : AttributesStringArray)
        {
            FArcweaveAttributeData AttributeAsset;
            // Iterate through the array of attributes and find with this id
            const TSharedPtr<FJsonObject>* AttributesObject;
            if (MainJsonObject->TryGetObjectField("attributes", AttributesObject))
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
                            // Extract the "cId", "name", and "cType"
                            AttributeAssetValueObject->TryGetStringField("cId", AttributeAsset.cId);
                            AttributeAssetValueObject->TryGetStringField("name", AttributeAsset.Name);
                            AttributeAssetValueObject->TryGetStringField("cType", AttributeAsset.cType);

                            // Parse "value" as FArcweaveAttributeValueData
                            const TSharedPtr<FJsonObject>* AttributeValueObject;
                            if (AttributeAssetValueObject->TryGetObjectField("value", AttributeValueObject))
                            {
                                ParseAttributeValue(*AttributeValueObject, AttributeAsset.Value);
                            }
                        }
                        ComponentAttributes.Add(AttributeAsset);
                    }
                }
            }
        }
    }
    return ComponentAttributes;
}

void UArcweaveSubsystem::ParseAttributeValue(const TSharedPtr<FJsonObject>& ValueObject, FArcweaveAttributeValueData& AttributeValue)
{
    ValueObject->TryGetStringField("data", AttributeValue.Data);
    ValueObject->TryGetStringField("type", AttributeValue.Type);
    ValueObject->TryGetBoolField("plain", AttributeValue.Plain);
}

TArray<FArcweaveBoardData> UArcweaveSubsystem::ParseBoard(const TSharedPtr<FJsonObject>& MainJsonObject)
{
    TArray<FArcweaveBoardData> Boards;
    const TSharedPtr<FJsonObject>* BoardsObject;
    if (MainJsonObject->TryGetObjectField("boards", BoardsObject))
    {
        for (const auto& BoardPair : BoardsObject->Get()->Values)
        {
            const TSharedPtr<FJsonObject> BoardValueObject = BoardPair.Value->AsObject();
            FArcweaveBoardData Board;
            Board.BoardId = BoardPair.Key;

            FString BoardName;
            if (BoardValueObject->TryGetStringField("name", BoardName))
            {
                Board.Name = BoardName;
            }

           Board.Elements = ParseElements(MainJsonObject, BoardValueObject);
           Board.Connections = ParseConnections(MainJsonObject, BoardValueObject);
           Boards.Add(Board);
        }
    }
    return Boards;
}

TArray<FArcweaveConnectionsData> UArcweaveSubsystem::ParseConnections(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& BoardValueObject)
{
      // Parse "connections" as an array of connection data structs
    TArray<FArcweaveConnectionsData> Connections;
    TArray<FString> ConnectionsArrayStrings;
    if (BoardValueObject->TryGetStringArrayField("connections", ConnectionsArrayStrings))
    {
        for (const FString& ConnectionId : ConnectionsArrayStrings)
        {
            // Extract connections
            const TSharedPtr<FJsonObject>* ConnectionsObject;
            if (MainJsonObject->TryGetObjectField("connections", ConnectionsObject))
            {
                for (const auto& ConnectionPair : ConnectionsObject->Get()->Values)
                {
                    if (ConnectionPair.Key == ConnectionId)
                    {
                        FArcweaveConnectionsData Connection;
                        Connection.Id = ConnectionPair.Key;
                        const TSharedPtr<FJsonObject> ConObject = ConnectionPair.Value->AsObject();

                        if (ConObject.IsValid())
                        {
                            ConObject->TryGetStringField("label", Connection.Label);
                            //UE_LOG(LogArcwarePlugin, Log, TEXT(" --- Connection object name: %s, label %s"), *ConnectionPair.Key, *Connection.Label);
                            ConObject->TryGetStringField("type", Connection.Type);
                            ConObject->TryGetStringField("theme", Connection.Theme);
                            ConObject->TryGetStringField("sourceid", Connection.Sourceid);
                            ConObject->TryGetStringField("targetid", Connection.Targetid);
                            ConObject->TryGetStringField("sourceType", Connection.SourceType);
                            ConObject->TryGetStringField("targetType", Connection.TargetType);
                            //LogStructFields(&Connection, FArcweaveConnectionsData::StaticStruct());
                        }

                        Connections.Add(Connection);
                    }
                }
            }
        }
    }
    return Connections;
}

TArray<FArcweaveElementData> UArcweaveSubsystem::ParseElements(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& BoardValueObject)
{
    TArray<FArcweaveElementData> Elements;
     // Parse "elements" as an array of element data structs
    TArray<FString> ElementArrayStrings;
    if (BoardValueObject->TryGetStringArrayField("elements", ElementArrayStrings))
    {
        for (const FString& ElementId : ElementArrayStrings)
        {
            // Extract elements
            const TSharedPtr<FJsonObject>* ElementsObject;
            if (MainJsonObject->TryGetObjectField("elements", ElementsObject))
            {
                for (const auto& ElementPair : ElementsObject->Get()->Values)
                {
                    if (ElementPair.Key == ElementId)
                    {
                        FArcweaveElementData Element;
                        Element.Id = ElementPair.Key;
                        const TSharedPtr<FJsonObject> ElementValueObject = ElementPair.Value->AsObject();

                        if (ElementValueObject.IsValid())
                        {
                            ElementValueObject->TryGetStringField("theme", Element.Theme);
                            ElementValueObject->TryGetStringField("title", Element.Title);
                            ElementValueObject->TryGetStringField("content", Element.Content);
                            ElementValueObject->TryGetStringArrayField("outputs", Element.Outputs);
                            Element.Components = ParseComponents(MainJsonObject, ElementValueObject);
                        }
                        Elements.Add(Element);
                    }
                }
            }
        }
    }
    return Elements;
}

TArray<FArcweaveComponentData> UArcweaveSubsystem::ParseComponents(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& ElementValueObject)
{
    // Parse "components" as an array of component data structs
    TArray<FArcweaveComponentData> Components;
    TArray<FString> ComponentsStringArray;
    if (ElementValueObject->TryGetStringArrayField("components", ComponentsStringArray))
    {
        for (const auto& ComponentId : ComponentsStringArray)
        {
            // Iterate through the array of components and find with this id
            const TSharedPtr<FJsonObject>* CompObject;
            if (MainJsonObject->TryGetObjectField("components", CompObject))
            {
                for (const auto& CompPair : CompObject->Get()->Values)
                {
                    if (CompPair.Key == ComponentId)
                    {
                        FArcweaveComponentData ElComponent;
                        ElComponent.Id = CompPair.Key;
                        const TSharedPtr<FJsonObject> ComponentValueObject = CompPair.Value->AsObject();

                        if (ComponentValueObject.IsValid())
                        {
                            // Extract the "name" and "root"
                            ComponentValueObject->TryGetStringField("name", ElComponent.Name);
                            ComponentValueObject->TryGetBoolField("root", ElComponent.Root);
                            ComponentValueObject->TryGetStringArrayField("children", ElComponent.Children);
                            ElComponent.Assets = ParseComponentAsset(ComponentValueObject);
                            ElComponent.Attributes = ParseComponentAttributes(MainJsonObject, ComponentValueObject);

                            Components.Add(ElComponent);
                        }
                    }
                }
            }
        }
    }                          
    return Components;
}

void UArcweaveSubsystem::ParseResponse(const FString& ResponseString)
{
    // Convert the response to a JSON object
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        // Failed to parse the response
        UE_LOG(LogArcwarePlugin, Error, TEXT("Failed to parse the HTTP Response!"));
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
            CoverObject->TryGetStringField("file", ProjectData.Cover.File);
            CoverObject->TryGetStringField("type", ProjectData.Cover.Type);
        }

        ProjectData.Boards = ParseBoard(JsonObject);
        OnArcweaveResponseReceived.Broadcast(ProjectData);
        //LogStructFieldsRecursive(&ProjectData, FArcweaveProjectData::StaticStruct(),0);
    }
    else
    {
        // Handle error here.
        UE_LOG(LogArcwarePlugin, Error, TEXT("Project name is invalid!"));
    }
}

void UArcweaveSubsystem::HandleFetch(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseString = Response->GetContentAsString();
        UE_LOG(LogArcwarePlugin, Log, TEXT("HTTP Response: %s"), *ResponseString);

        ParseResponse(ResponseString);
    }
    else
    {
        // Handle error here.
        UE_LOG(LogArcwarePlugin, Error, TEXT("HTTP Request failed!"));
    }
}

void  UArcweaveSubsystem::LogStructFields(const void* StructPtr, UStruct* StructDefinition)
{
    if (!StructPtr || !StructDefinition)
    {
        UE_LOG(LogArcwarePlugin, Warning, TEXT("Invalid struct or struct definition provided!"));
        return;
    }

    // Iterate through all fields of the struct
    for (TFieldIterator<FProperty> PropertyIt(StructDefinition); PropertyIt; ++PropertyIt)
    {
        FProperty* Property = *PropertyIt;
        FString PropertyName = Property->GetName();

        // Use the Property's accessors to get the value as a string
        FString PropertyValue;
        Property->ExportText_InContainer(0, PropertyValue, StructPtr, StructPtr, nullptr, PPF_None);

        // Log the property name and value
        UE_LOG(LogArcwarePlugin, Log, TEXT("%s: %s"), *PropertyName, *PropertyValue);
    }
}

void UArcweaveSubsystem::LogStructFieldsRecursive(const void* StructPtr, UStruct* StructDefinition, int32 IndentationLevel)
{
    if (!StructPtr || !StructDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid struct or struct definition provided!"));
        return;
    }

    // Create a string for indentation based on the current level
    FString Indentation = FString::ChrN(IndentationLevel, TEXT('\t'));

    // Iterate through all fields of the struct
    for (TFieldIterator<FProperty> PropertyIt(StructDefinition); PropertyIt; ++PropertyIt)
    {
        FProperty* Property = *PropertyIt;
        FString PropertyName = Property->GetName();

        // Use the Property's accessors to get the value as a string
        FString PropertyValue;
        Property->ExportText_InContainer(0, PropertyValue, StructPtr, StructPtr, nullptr, PPF_None);

        // Log the property name and value with indentation on a new line
        FString LogMessage = FString::Printf(TEXT("%s%s: %s\n"), *Indentation, *PropertyName, *PropertyValue);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);

        // If the property is a nested struct, recursively log its fields with increased indentation
        if (Property->IsA(FStructProperty::StaticClass()))
        {
            FStructProperty* StructProperty = Cast<FStructProperty>(Property);
            if (StructProperty)
            {
                const void* NestedStructPtr = StructProperty->ContainerPtrToValuePtr<void>(StructPtr);
                UStruct* NestedStructDefinition = StructProperty->Struct;

                // Recursively log the fields of the nested struct with increased indentation
                LogStructFieldsRecursive(NestedStructPtr, NestedStructDefinition, IndentationLevel + 1);
            }
        }
    }
}
