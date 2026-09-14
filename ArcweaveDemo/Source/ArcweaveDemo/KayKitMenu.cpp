#include "KayKitController.h"
#include "ArcweaveSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"

void AKayKitController::LoadStory(bool Local, const FString& Token, const FString& Hash)
{
    const FString CleanToken=Token.TrimStartAndEnd();
    const FString CleanHash=Hash.TrimStartAndEnd();
    if (!Local && (CleanToken.IsEmpty() || CleanHash.IsEmpty()))
    {
        StatusMessage=TEXT("Enter an API token and project hash.");
        return;
    }
    EndDialogue();
    if (!bImportRequested) bStoryReadyBeforeImport = bStoryReady;
    bStoryReady=false;
    bImportRequested=true;
    StatusMessage=TEXT("Loading Arcweave project...");
    if (bMenuOpen) ToggleMenu();
    // FetchData reloads saved settings, which can override the source selected in this menu.
    // Use the explicit source methods and keep credentials out of the config files.
    if (Local)
    {
        if (!Arcweave->LoadJsonFile())
        {
            OnFetchCompleted(false, TEXT("Unable to load the bundled project JSON. The previous project was kept."));
            return;
        }
    }
    else
    {
        SessionAPIToken=CleanToken;
        SessionProjectHash=CleanHash;
        Arcweave->FetchDataFromAPI(CleanToken,CleanHash);
    }
}

void AKayKitController::OnFetchCompleted(bool bSuccess, const FString& Message)
{
    if (bSuccess || !bImportRequested) return;
    bImportRequested = false;
    bStoryReady = bStoryReadyBeforeImport;
    StatusMessage = Message;
    if (!bMenuOpen) ToggleMenu();
}
void AKayKitController::ToggleMenu()
{
    if (bInDialogue) return;
    bMenuOpen=!bMenuOpen;
    SetUIInput(bMenuOpen);
    if (!bMenuOpen)
    {
        if (MenuWidget) GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
        MenuWidget.Reset(); return;
    }
    const auto Token=SNew(SEditableTextBox).HintText(FText::FromString(TEXT("API token"))).IsPassword(true)
        .Text(FText::FromString(SessionAPIToken))
        .OnTextChanged_Lambda([this](const FText& Text){SessionAPIToken=Text.ToString();});
    const auto Hash=SNew(SEditableTextBox).HintText(FText::FromString(TEXT("Project hash")))
        .Text(FText::FromString(SessionProjectHash))
        .OnTextChanged_Lambda([this](const FText& Text){SessionProjectHash=Text.ToString();});
    MenuWidget=SNew(SVerticalBox)
    + SVerticalBox::Slot().FillHeight(1)
    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
    [ SNew(SBorder).Padding(28).BorderBackgroundColor(FLinearColor(.04f,.04f,.04f,.98f))
      [ SNew(SBox).WidthOverride(460)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)[SNew(STextBlock).Text(FText::FromString(TEXT("Arcweave project"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",24))]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Token]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Hash]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(STextBlock).Text_Lambda([this](){return FText::FromString(StatusMessage);}).AutoWrapText(true)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,8)[SNew(SButton).Text(FText::FromString(TEXT("Import from Arcweave"))).OnClicked_Lambda([this,Token,Hash](){LoadStory(false,Token->GetText().ToString(),Hash->GetText().ToString()); return FReply::Handled();})]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(SButton).Text(FText::FromString(TEXT("Load bundled example JSON"))).OnClicked_Lambda([this](){LoadStory(true);return FReply::Handled();})]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(SButton).Text(FText::FromString(TEXT("Resume"))).OnClicked_Lambda([this](){ToggleMenu();return FReply::Handled();})]
          + SVerticalBox::Slot().AutoHeight().Padding(0,12,0,0)[SNew(STextBlock).Text(FText::FromString(TEXT("WASD: move   Mouse: look   E: talk   Esc: menu")))]
        ]
      ]
    ]
    + SVerticalBox::Slot().FillHeight(1);
    GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef(),10);
}
