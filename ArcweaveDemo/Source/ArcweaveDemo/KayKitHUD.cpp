#include "KayKitHUD.h"
#include "KayKitController.h"
#include "KayKitCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "CanvasItem.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/SlateFontInfo.h"

namespace
{
FSlateFontInfo HudFont(float Size)
{
    // Rasterize each glyph at its displayed size. Scaling a small cached glyph
    // stretches its pixels, especially at larger viewport resolutions.
    return FSlateFontInfo(GEngine->GetMediumFont(), FMath::Max(1, FMath::RoundToInt(14.5f * Size)), "Regular");
}
}

void AKayKitHUD::DrawLabel(const FString& Text, float X, float Y, float Size, FLinearColor Color)
{
    FCanvasTextItem Item(FVector2D(FMath::RoundToFloat(X), FMath::RoundToFloat(Y)), FText::FromString(Text), HudFont(Size), Color);
    Canvas->DrawItem(Item);
}

FVector2D AKayKitHUD::MeasureLabel(const FString& Text, float Size) const
{
    const auto FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    return FontMeasure->Measure(Text, HudFont(Size), Canvas->Canvas->GetDPIScale());
}

void AKayKitHUD::ChangeVariablePage(int32 Direction)
{
    VariablePage = FMath::Clamp(VariablePage + Direction, 0, VariablePageCount - 1);
}

void AKayKitHUD::DrawVariables(AKayKitController* Controller, float Scale)
{
    const FVector2D ViewportSize(Canvas->ClipX, Canvas->ClipY);
    const float TextScale = .85f * Scale;
    if (CachedVariableRevision != Controller->GetVariableDisplayRevision() || CachedViewportSize != ViewportSize)
    {
        CachedVariableRevision = Controller->GetVariableDisplayRevision();
        CachedViewportSize = ViewportSize;
        ClippedVariableLines = Controller->GetVariableDisplayLines();
        if (ClippedVariableLines.IsEmpty()) ClippedVariableLines.Add(TEXT("No variables"));
        VariablePanelWidth = 238 * Scale;
        const float MaximumWidth = 400 * Scale;
        for (const FString& Line : ClippedVariableLines)
        {
            const float Width = MeasureLabel(Line, TextScale).X;
            VariablePanelWidth = FMath::Clamp(Width + 24 * Scale, VariablePanelWidth, MaximumWidth);
        }
        const float TextWidth = VariablePanelWidth - 24 * Scale;
        for (FString& Line : ClippedVariableLines)
        {
            const float Width = MeasureLabel(Line, TextScale).X;
            if (Width <= TextWidth) continue;
            // Binary search keeps long imported strings cheap to fit. Layout is
            // rebuilt only when the data or viewport size changes.
            int32 Low = 0, High = Line.Len();
            while (Low < High)
            {
                const int32 Middle = (Low + High + 1) / 2;
                const float CandidateWidth = MeasureLabel(Line.Left(Middle) + TEXT("..."), TextScale).X;
                if (CandidateWidth <= TextWidth) Low = Middle;
                else High = Middle - 1;
            }
            Line = Line.Left(Low) + TEXT("...");
        }
    }
    const int32 RowsPerPage = FMath::Max(1, FMath::FloorToInt((Canvas->ClipY * .5f - 65 * Scale) / (19 * Scale)));
    VariablePageCount = FMath::Max(1, FMath::DivideAndRoundUp(ClippedVariableLines.Num(), RowsPerPage));
    VariablePage = FMath::Clamp(VariablePage, 0, VariablePageCount - 1);
    const int32 Start = VariablePage * RowsPerPage;
    const int32 Rows = FMath::Min(RowsPerPage, ClippedVariableLines.Num() - Start);
    const float X = Canvas->ClipX - 25 * Scale - VariablePanelWidth;
    const float Height = (45 + 19 * Rows + (VariablePageCount > 1 ? 22 : 0)) * Scale;
    DrawRect(FLinearColor(.035f, .04f, .045f, .65f), X, 24 * Scale, VariablePanelWidth, Height);
    DrawLabel(TEXT("Variables:"), X + 10 * Scale, 29 * Scale, 1.15f * Scale);
    for (int32 Row = 0; Row < Rows; ++Row)
        DrawLabel(ClippedVariableLines[Start + Row], X + 12 * Scale, (55 + Row * 19) * Scale, TextScale);
    if (VariablePageCount > 1)
    {
        const FString Footer = FString::Printf(TEXT("%d / %d   PgUp / PgDn"), VariablePage + 1, VariablePageCount);
        DrawLabel(Footer, X + 12 * Scale, (58 + Rows * 19) * Scale, .75f * Scale, FLinearColor(.7f, .75f, .8f));
    }
}

void AKayKitHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* PC=Cast<AKayKitController>(PlayerOwner);
    if (!PC || !Canvas) return;
    const float Scale=FMath::Min(Canvas->ClipX/1280.f,Canvas->ClipY/720.f);
    auto Text=[&](const FString& Value,float X,float Y,float Size,FLinearColor Color=FLinearColor::White)
    { DrawLabel(Value,X,Y,Size*Scale,Color); };
    Text(TEXT("Press Esc to open menu"),34*Scale,20*Scale,1);
    DrawVariables(PC, Scale);
    for (TActorIterator<AKayKitCharacter> It(GetWorld()); It; ++It)
    {
        if (!PC->HasVariable(It->HealthVariable)) continue;
        FVector2D Screen;
        if (!PC->ProjectWorldLocationToScreen(It->GetActorLocation()+FVector(0,0,140),Screen)) continue;
        if (Screen.X<0 || Screen.X>Canvas->ClipX || Screen.Y<0 || Screen.Y>Canvas->ClipY) continue;
        const float Health=PC->VariableNumber(It->HealthVariable,100);
        const float Width=120*Scale;
        DrawRect(FLinearColor(.1,.1,.1,.85),Screen.X-Width/2,Screen.Y,Width,8*Scale);
        DrawRect(It->HealthColor,Screen.X-Width/2,Screen.Y,Width*FMath::Clamp(Health/100,0.f,1.f),8*Scale);
        Text(FString::Printf(TEXT("%.0f"),Health),Screen.X-12*Scale,Screen.Y-28*Scale,1.1);
    }
    if (!PC->StatusMessage.IsEmpty()) Text(PC->StatusMessage,28*Scale,50*Scale,1.1);
    if (PC->NearbyNPC && !PC->bInDialogue && !PC->bMenuOpen)
    {
        FVector2D Screen;
        if (PC->ProjectWorldLocationToScreen(PC->NearbyNPC->GetActorLocation()+FVector(0,0,105),Screen))
            Text(TEXT("Press E to talk"),Screen.X-80*Scale,Screen.Y,1.2);
    }
    if (!PC->bInDialogue) return;
    const float PanelWidth=828*Scale;
    const float X=154*Scale;
    const float ButtonHeight=52*Scale;
    TArray<FString> Lines,Paragraphs;
    PC->DialogueText.ParseIntoArray(Paragraphs,TEXT("\n"),false);
    const float TextWidth=PanelWidth-24*Scale;
    for (const FString& Paragraph : Paragraphs)
    {
        TArray<FString> Words; Paragraph.ParseIntoArray(Words,TEXT(" "),true);
        FString Line;
        for (const FString& Word : Words)
        {
            const float W=MeasureLabel(Line.IsEmpty()?Word:Line+TEXT(" ")+Word,Scale).X;
            if (W>TextWidth && !Line.IsEmpty()) { Lines.Add(Line); Line=Word; }
            else Line+=(Line.IsEmpty()?TEXT(""):TEXT(" "))+Word;
        }
        Lines.Add(Line);
    }
    const float ContentHeight=FMath::Max(108.f*Scale,(Lines.Num()*24+20)*Scale);
    const int32 Count=FMath::Max(1,PC->Choices.Num());
    float Y=Canvas->ClipY-126*Scale-ContentHeight;
    DrawRect(FLinearColor(.018,.025,.04,.55),X,Y,PanelWidth,ContentHeight);
    if (!PC->PortraitName.IsEmpty())
    {
        if (!Portraits.Contains(PC->PortraitName))
        {
            const FString Path=TEXT("/Game/KayKit/UI/")+PC->PortraitName+TEXT(".")+PC->PortraitName;
            Portraits.Add(PC->PortraitName,LoadObject<UTexture2D>(nullptr,*Path));
        }
        if (auto* Portrait=Portraits[PC->PortraitName].Get()) DrawTexture(Portrait,34*Scale,Y,108*Scale,108*Scale,0,0,1,1);
    }
    for (int32 Index=0;Index<Lines.Num();++Index) Text(Lines[Index],X+12*Scale,Y+(10+24*Index)*Scale,1);
    Y=Canvas->ClipY-99*Scale;
    float MouseX,MouseY; PC->GetMousePosition(MouseX,MouseY);
    for (int32 Index=0;Index<Count;++Index)
    {
        const float Width=FMath::Min(265.f*Scale,(PanelWidth-16*Scale*(Count-1))/Count);
        const float ButtonX=X+PanelWidth-Width-(Width+16*Scale)*Index;
        const bool Hover=MouseX>=ButtonX && MouseX<ButtonX+Width && MouseY>=Y && MouseY<Y+ButtonHeight;
        DrawRect(Hover?FLinearColor(.08,.1,.12,.96):FLinearColor(.006,.008,.01,.94),ButtonX,Y,Width,ButtonHeight);
        FString Label=PC->Choices.IsValidIndex(Index)?PC->Choices[Index].Label:TEXT("Continue");
        if (Label==TEXT("Next")) Label=TEXT("Continue");
        const float TW=MeasureLabel(Label,.9f*Scale).X;
        const float TextScale=FMath::Min(.9f,(Width-20*Scale)/FMath::Max(TW,1.f)*.9f);
        Text(Label,ButtonX+10*Scale,Y+18*Scale,TextScale);
        AddHitBox(FVector2D(ButtonX,Y),FVector2D(Width,ButtonHeight),FName(*FString::FromInt(Index)),true);
    }
}
void AKayKitHUD::NotifyHitBoxClick(FName BoxName)
{
    if (auto* PC=Cast<AKayKitController>(PlayerOwner)) PC->Choose(FCString::Atoi(*BoxName.ToString()));
}
