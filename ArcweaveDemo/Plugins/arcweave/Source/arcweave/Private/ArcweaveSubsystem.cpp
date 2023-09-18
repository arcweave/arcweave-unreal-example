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

FArcweaveAPISettings UArcweaveSubsystem::LoadArcweaveSettings() const
{
	FArcweaveAPISettings OutSetttings = FArcweaveAPISettings();	
	/*UArcweaveSettings* ArcweaveSettings = GetMutableDefault<UArcweaveSettings>();
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
	}*/

    UArcweaveSettings* ArcweaveSettings = GetMutableDefault<UArcweaveSettings>();
    if (ArcweaveSettings)
    {
        ArcweaveSettings->ReloadConfig();
        if (GConfig)
        {
            if(GConfig->GetString(ARCWEAVE_SETTINGS_SECTION, TEXT("APIToken"), OutSetttings.APIToken, GGameIni))
            {
                UE_LOG(LogTemp, Warning, TEXT("Read APIToken: %s"), *OutSetttings.APIToken);
            }

            if(GConfig->GetString(ARCWEAVE_SETTINGS_SECTION, TEXT("Hash"), OutSetttings.Hash, GGameIni))
            {
                UE_LOG(LogTemp, Warning, TEXT("Read Hash: %s"), *OutSetttings.Hash);
            }
        }
    }



	return OutSetttings;
}

void UArcweaveSubsystem::SaveArcweaveSettings(const FString& APIToken, const FString& ProjectHash)
{
    if (GConfig == nullptr)
    {
        return;
    }

    //GConfig->EmptySection(TEXT("TargetDeviceServices"), GGameIni);

    // save configuration
    //GConfig->SetString(ARCWEAVE_SETTINGS_SECTION, TEXT("APIToken"), *APIToken, GGameIni);
    //GConfig->SetString(ARCWEAVE_SETTINGS_SECTION, TEXT("Hash"), *ProjectHash, GGameIni);
    //GConfig->Flush(false, GGameIni);

    UArcweaveSettings* ArcweaveSettings = GetMutableDefault<UArcweaveSettings>();
    if (ArcweaveSettings)
    {
        ArcweaveSettings->APIToken = APIToken;
        ArcweaveSettings->Hash = ProjectHash;
        ArcweaveSettings->SaveConfig();
    }
}

FString UArcweaveSubsystem::RemoveHtmlTags(const FString& InputString)
{
    FString CleanedString = InputString;

    // Search for the opening '<' character in the string
    int32 TagStartIndex = CleanedString.Find(TEXT("<"));
    
    while (TagStartIndex != INDEX_NONE)
    {
        int32 TagEndIndex = CleanedString.Find(TEXT(">"), ESearchCase::IgnoreCase, ESearchDir::FromStart, TagStartIndex);

        // If both '<' and '>' are found, remove the tag
        if (TagEndIndex != INDEX_NONE)
        {
            CleanedString.RemoveAt(TagStartIndex, TagEndIndex - TagStartIndex + 1, /*bAllowShrinking*/ true);
            // No need to adjust the TagStartIndex since we've just removed a segment
        }
        else
        {
            // If no matching '>' is found or the string is finished, break the loop
            break;
        }

        TagStartIndex = CleanedString.Find(TEXT("<"));
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
            
           Board.Visits = InitVisist(BoardValueObject, Board);
           Board.Elements = ParseElements(MainJsonObject, BoardValueObject, Board);
           Board.Connections = ParseConnections(FString("connections"), MainJsonObject, BoardValueObject);
           Boards.Add(Board);
        }
    }
    return Boards;
}

TMap<FString, FArcweaveVariable> UArcweaveSubsystem::ParseVariables(const TSharedPtr<FJsonObject>& MainJsonObject)
{
    TMap<FString, FArcweaveVariable> InitialVars;
    const TSharedPtr<FJsonObject>* VariablesObject;
    if (MainJsonObject->TryGetObjectField("variables", VariablesObject))
    {
        for (const auto& VariablePair : VariablesObject->Get()->Values)
        {
            const TSharedPtr<FJsonObject> VarObject = VariablePair.Value->AsObject();
            bool isRoot = false;
            if (!(VarObject->TryGetBoolField("root", isRoot) && isRoot)) {
                FArcweaveVariable Variable;
                Variable.Id = VariablePair.Key;
                VarObject->TryGetStringField("name", Variable.Name);
                VarObject->TryGetStringField("type", Variable.Type);
                if (Variable.Type == "string") {
                    Variable.Value = VarObject->GetStringField("value");
                }
                else if (Variable.Type == "integer") {
                    Variable.Value.AppendInt(VarObject->GetIntegerField("value"));
                }
                else if (Variable.Type == "boolean") {
                    Variable.Value = FString::Printf(TEXT("%s"), VarObject->GetBoolField("value") ? TEXT("true") : TEXT("false"));
                }
                else if (Variable.Type == "float")
                {
                    Variable.Value = FString::SanitizeFloat(VarObject->GetNumberField("value"));
                }
                InitialVars.Add(Variable.Id, Variable);
            }
        }
    }
    return InitialVars;
}

TArray<FArcweaveConnectionsData> UArcweaveSubsystem::ParseConnections(const FString& FieldName, const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& ParentValueObject)
{
      // Parse "connections" as an array of connection data structs
    TArray<FArcweaveConnectionsData> Connections;
    TArray<FString> ConnectionsArrayStrings;
    if (ParentValueObject->TryGetStringArrayField(FieldName, ConnectionsArrayStrings))
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
                            FString DirtyLabel = FString("");
                            ConObject->TryGetStringField("label", DirtyLabel);
                            Connection.Label = RemoveHtmlTags(DirtyLabel);                            
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

TArray<FArcweaveElementData> UArcweaveSubsystem::ParseElements(const TSharedPtr<FJsonObject>& MainJsonObject, const TSharedPtr<FJsonObject>& BoardValueObject, const FArcweaveBoardData& BoardObj)
{
    TArray<FArcweaveElementData> Elements;
     // Parse "elements" as an array of element data structs
    TArray<FString> ElementArrayStrings;
    if (BoardValueObject->TryGetStringArrayField("elements", ElementArrayStrings))
    {
        TMap<FString, int>  Visits;
        for (const FString& ElementId : ElementArrayStrings)
        {
            Visits.Add(ElementId,0);
        }
        //then search for the element pairs
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
                            //inti visits object for the transpiler
                            
                            //get the values from the json object
                            ElementValueObject->TryGetStringField("theme", Element.Theme);
                            FString DirtyTitle = FString("");
                            ElementValueObject->TryGetStringField("title", DirtyTitle);
                            Element.Title = RemoveHtmlTags(DirtyTitle);
                            FString DirtyContent = FString("");
                            ElementValueObject->TryGetStringField("content", DirtyContent);
                            //Element.Content = RemoveHtmlTags(DirtyContent);
                            FArcscriptTranspilerOutput Output = RunTranspiler(DirtyContent, Element.Id, ProjectData.InitialVars, Visits);
                            Element.Outputs = ParseConnections(FString("outputs"), MainJsonObject, ElementValueObject);
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

TMap<FString, int> UArcweaveSubsystem::InitVisist(const TSharedPtr<FJsonObject>& BoardValueObject, FArcweaveBoardData& BoardObj)
{
    //add the visits to the board data
    TMap<FString, int> AllVisits;
    TArray<FString> ElementArrayStrings;
    if (BoardValueObject->TryGetStringArrayField("elements", ElementArrayStrings))
    {
        for (const FString& ElementId : ElementArrayStrings)
        {
           AllVisits.Add(ElementId, 0);
        }
    }
    return AllVisits;
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

TArray<FArcweaveComponentData> UArcweaveSubsystem::ParseAllComponents(const TSharedPtr<FJsonObject>& MainJsonObject)
{
    // Parse "components" as an array of component data structs
    TArray<FArcweaveComponentData> Components;
    // Iterate through the array of components and find with this id
    const TSharedPtr<FJsonObject>* CompObject;
    if (MainJsonObject->TryGetObjectField("components", CompObject))
    {
        for (const auto& CompPair : CompObject->Get()->Values)
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
    if (JsonObject->TryGetStringField("name", ProjectData.Name))
    {
        if (JsonObject->HasField("cover"))
        {
            TSharedPtr<FJsonObject> CoverObject = JsonObject->GetObjectField("cover");

            // Extract cover data
            CoverObject->TryGetStringField("file", ProjectData.Cover.File);
            CoverObject->TryGetStringField("type", ProjectData.Cover.Type);
        }        

        ProjectData.InitialVars = ParseVariables(JsonObject);
        ProjectData.Boards = ParseBoard(JsonObject);
        ProjectData.Components = ParseAllComponents(JsonObject);
        OnArcweaveResponseReceived.Broadcast(ProjectData);
        //LogStructFieldsRecursive(&ProjectData, FArcweaveProjectData::StaticStruct(),0);
    }
    else
    {
        // Handle error here.
        UE_LOG(LogArcwarePlugin, Error, TEXT("Project name is invalid!"));
    }
}

FArcscriptTranspilerOutput UArcweaveSubsystem::RunTranspiler(FString Code, FString ElementId,
    TMap<FString, FArcweaveVariable> InitialVars, TMap<FString, int> Visits)
{
    FArcscriptTranspilerOutput Output;
    if (ArcscriptWrapper)
    {
        Output = ArcscriptWrapper->RunScript(Code, ElementId, InitialVars, Visits);
    }
    
    return Output;
}

void UArcweaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FarcweaveModule* arcweaveModule = FModuleManager::GetModulePtr<FarcweaveModule>("Arcweave");

    ArcscriptWrapper = arcweaveModule->getArcscriptWrapper();
    // we must read from engine config here
    FArcweaveAPISettings ArcweaveAPISettings = LoadArcweaveSettings();
    FetchData(FString(ArcweaveAPISettings.APIToken), ArcweaveAPISettings.Hash);
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



/*void  UArcweaveSubsystem::LogStructFields(const void* StructPtr, UStruct* StructDefinition)
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
}*/

/*void UArcweaveSubsystem::LogStructFieldsRecursive(const void* StructPtr, UStruct* StructDefinition, int32 IndentationLevel)
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
}*/
