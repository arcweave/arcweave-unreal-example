#include "KayKitController.h"
#include "KayKitCharacter.h"
#include "ArcweaveSubsystem.h"
#include "GetIsTargetBranchOutput.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"

namespace
{
bool HasMarker(const FArcweaveElementData& Element, const FString& Marker)
{
    for (const auto& Component : Element.Components)
        if (Component.Name.Equals(Marker, ESearchCase::IgnoreCase)) return true;
    for (const auto& Attribute : Element.Attributes)
        if (Attribute.Value.Data.Equals(Marker, ESearchCase::IgnoreCase)) return true;
    return false;
}
bool HasScript(const FString& Text) { return Text.Contains(TEXT("<code")) || Text.Contains(TEXT("<pre")); }
FString QualifiedVariableName(const FArcweaveVariable& Variable)
{
    return Variable.Scope.IsEmpty() ? Variable.Name : Variable.Scope+TEXT(".")+Variable.Name;
}
}

FString AKayKitController::PlainText(FString Text)
{
    Text.ReplaceInline(TEXT("</p>"),TEXT("\n"));
    Text.ReplaceInline(TEXT("<br>"),TEXT("\n"));
    FString Result;
    bool InTag=false;
    for (TCHAR C : Text) { if (C=='<') InTag=true; else if (C=='>') InTag=false; else if (!InTag) Result.AppendChar(C); }
    Result.ReplaceInline(TEXT("&amp;"),TEXT("&")); Result.ReplaceInline(TEXT("&quot;"),TEXT("\""));
    Result.ReplaceInline(TEXT("&#39;"),TEXT("'")); Result.ReplaceInline(TEXT("&nbsp;"),TEXT(" "));
    Result.ReplaceInline(TEXT("&lt;"),TEXT("<")); Result.ReplaceInline(TEXT("&gt;"),TEXT(">"));
    return Result.TrimStartAndEnd();
}
FString AKayKitController::VariableValue(const FString& Name) const
{
    const FString* Id = VariableIdsByName.Find(Name);
    const FArcweaveVariable* Variable = Id ? RuntimeVariables.Find(*Id) : nullptr;
    return Variable ? Variable->Value : FString();
}

bool AKayKitController::HasVariable(const FString& Name) const
{
    return VariableIdsByName.Contains(Name);
}

TArray<FString> AKayKitController::VariableNames() const
{
    return SortedVariableNames;
}

float AKayKitController::VariableNumber(const FString& Name, float Fallback) const
{
    const FString Value = VariableValue(Name);
    float Number = Fallback;
    return LexTryParseString(Number, *Value) ? Number : Fallback;
}

void AKayKitController::RebuildVariableDisplay()
{
    VariableIdsByName.Reset();
    for (const auto& Pair : RuntimeVariables)
        VariableIdsByName.Add(QualifiedVariableName(Pair.Value), Pair.Key);
    VariableIdsByName.GenerateKeyArray(SortedVariableNames);
    SortedVariableNames.Sort();
    VariableDisplayLines.Reset(SortedVariableNames.Num());
    for (const FString& Name : SortedVariableNames)
    {
        FString Line = Name + TEXT(": ") + VariableValue(Name);
        Line.ReplaceInline(TEXT("\r"), TEXT(" "));
        Line.ReplaceInline(TEXT("\n"), TEXT(" "));
        Line.ReplaceInline(TEXT("\t"), TEXT(" "));
        VariableDisplayLines.Add(MoveTemp(Line));
    }
    ++VariableDisplayRevision;
}

FString AKayKitController::AttributeValue(const FArcweaveAttributeData& Attribute) const
{
    const FArcweaveVariable* Variable = RuntimeVariables.Find(Attribute.Id);
    return Variable ? Variable->Value : Attribute.Value.Data;
}

void AKayKitController::ApplySceneAttributes()
{
    // Restore scene defaults first, so importing a project without these attributes
    // cannot keep another project's weather, sky or character colors.
    bRainEnabled = false;
    FLinearColor SkyColor(.24f, .30f, .33f);
    for (const auto& Pair : DefaultHealthColors)
        if (Pair.Key.IsValid()) Pair.Key->HealthColor = Pair.Value;

    for (const FArcweaveComponentData& Component : SceneComponents)
    {
        const bool bEnvironment = Component.CustomId == TEXT("environment") || Component.Name == TEXT("Environment");
        for (const FArcweaveAttributeData& Attribute : Component.Attributes)
        {
            const FString Value = PlainText(AttributeValue(Attribute));
            if (bEnvironment)
            {
                if (Attribute.CustomId == TEXT("weather") || Attribute.Name == TEXT("Weather"))
                    bRainEnabled = Value.Equals(TEXT("Rain"), ESearchCase::IgnoreCase);
                if (Attribute.CustomId == TEXT("time") || Attribute.Name == TEXT("Time"))
                    SkyColor = Value.Equals(TEXT("Night"), ESearchCase::IgnoreCase) ? FLinearColor::Black : FLinearColor(.24f, .30f, .33f);
            }
            if (Attribute.CustomId == TEXT("color") || Attribute.Name == TEXT("Color"))
            {
                const FString Lower = Value.ToLower();
                const FLinearColor Color = Lower == TEXT("red") ? FLinearColor::Red
                    : Lower == TEXT("blue") ? FLinearColor(.15f, .35f, 1.f)
                    : Lower == TEXT("green") ? FLinearColor::Green
                    : FLinearColor(FColor::FromHex(Value));
                for (const auto& Pair : DefaultHealthColors)
                    if (Pair.Key.IsValid() && Pair.Key->CharacterName == Component.Name)
                        Pair.Key->HealthColor = Color;
            }
            if (Component.Name.Equals(TEXT("attack"), ESearchCase::IgnoreCase) &&
                Attribute.Name.Equals(TEXT("SwingSpeed"), ESearchCase::IgnoreCase))
                SwingSpeed = FMath::Max(.1f, FCString::Atof(*Value));
        }
    }
    if (Rain) Rain->SetVisibility(bRainEnabled);
    if (BackgroundMaterial) BackgroundMaterial->SetVectorParameterValue(TEXT("SkyColor"), SkyColor);
}

void AKayKitController::OnProjectLoaded(const FArcweaveProjectData& Data)
{
    RuntimeVariables = Data.CurrentVars;
    SceneComponents = Data.Components;
    RebuildVariableDisplay();
    bStoryReady = Data.Boards.Num() > 0;
    StatusMessage = bStoryReady ? TEXT("") : TEXT("No story boards found. Press Esc to import a project.");
    if (bStoryReady && bImportRequested)
        StatusMessage = FString::Printf(TEXT("Loaded %s (%d boards)."), *Data.Name, Data.Boards.Num());
    bImportRequested = false;
    bCanAttack = false;
    bPotionCollected = false;
    SwingSpeed = 1.f;
    OnVariablesChanged({});
    UE_LOG(LogTemp, Display, TEXT("KayKit: Loaded %s (%d boards)"), *Data.Name, Data.Boards.Num());
}

void AKayKitController::OnVariablesChanged(const TArray<FArcweaveVariable>& Variables)
{
    for (const auto& Variable : Variables) RuntimeVariables.Add(Variable.Id, Variable);
    if (!Variables.IsEmpty()) RebuildVariableDisplay();
    ApplySceneAttributes();
    if (HasVariable(TEXT("sword_locked"))) bCanAttack = VariableValue(TEXT("sword_locked")) == TEXT("false");
    bPotionCollected |= VariableValue(TEXT("have_potion")) == TEXT("true");
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        if (It->ActorHasTag(TEXT("ArcweavePotion"))) It->SetActorHiddenInGame(bPotionCollected);
}

FArcweaveBoardData AKayKitController::FindBoardFor(const FString& Id) const
{
    for (const auto& Board : Arcweave->GetArcweaveProjectData().Boards)
    {
        for (const auto& Element : Board.Elements) if (Element.Id==Id) return Board;
        for (const auto& Branch : Board.Branches) if (Branch.Id==Id) return Board;
        for (const auto& Jumper : Board.Jumpers) if (Jumper.Id==Id) return Board;
    }
    return {};
}
bool AKayKitController::StartDialogue(AKayKitCharacter* NPC)
{
    if (!bStoryReady || !NPC || bMenuOpen) return false;
    for (const auto& Board : Arcweave->GetArcweaveProjectData().Boards)
    {
        if (Board.CustomId!=NPC->DialogueBoard && Board.Name!=NPC->DialogueBoard) continue;
        for (const auto& Element : Board.Elements)
        {
            if (!HasMarker(Element,TEXT("dialogue_start"))) continue;
            ActiveNPC=NPC; bInDialogue=true;
            if (auto* ControlledPawn=Cast<ACharacter>(GetPawn())) ControlledPawn->GetCharacterMovement()->StopMovementImmediately();
            SetUIInput(true);
            EnterElement(Element.Id);
            return true;
        }
    }
    StatusMessage=FString::Printf(TEXT("No dialogue_start element in %s"), *NPC->DialogueBoard);
    return false;
}
void AKayKitController::EndDialogue()
{
    bInDialogue=false; ActiveNPC=nullptr; Choices.Reset(); DialogueText.Empty(); Speaker.Empty();
    SetUIInput(false);
}
void AKayKitController::EnterElement(const FString& Id, int32 Depth)
{
    if (Depth>48) { StatusMessage=TEXT("Story contains a navigation loop."); EndDialogue(); return; }
    bool Success=false;
    const FArcweaveElementData Element=Arcweave->TranspileObject(Id,Success);
    if (!Success) { StatusMessage=TEXT("Unable to evaluate this dialogue element."); EndDialogue(); return; }
    CurrentElementId=Id;
    bEndElement=HasMarker(Element,TEXT("dialogue_end"));
    if (ActiveNPC)
        if (auto* Sound=ActiveNPC->DialogueSounds.Find(Id)) UGameplayStatics::PlaySound2D(this,Sound->Get());
    Speaker.Empty(); PortraitName.Empty();
    for (const auto& Component : Element.Components)
    {
        if (Component.Name.Equals(TEXT("attack"),ESearchCase::IgnoreCase))
        {
            bCanAttack=true;
            for (const auto& Attr : Component.Attributes) if (Attr.Name.Equals(TEXT("SwingSpeed"),ESearchCase::IgnoreCase)) SwingSpeed=FCString::Atof(*AttributeValue(Attr));
        }
        if (Component.Name==TEXT("Drake Miles")) { Speaker=Component.Name; PortraitName=TEXT("player"); }
        else if (Component.Name==TEXT("Wanda Elandor")) { Speaker=Component.Name; PortraitName=TEXT("wanda"); }
        else if (Component.Name==TEXT("Healer")) { Speaker=Component.Name; PortraitName=TEXT("healer"); }
        else if (Component.Name==TEXT("Fighter")) { Speaker=Component.Name; PortraitName=TEXT("fighter"); }
    }
    DialogueText=PlainText(Element.Content);
    Choices.Reset();
    for (const auto& Connection : Element.Outputs) ExpandChoice(Connection,TEXT(""));
    if (DialogueText.IsEmpty() && !bEndElement && Choices.Num()==1 && Choices[0].Label==TEXT("Next")) { FollowConnection(Choices[0].Connection,Depth+1); return; }
    UE_LOG(LogTemp, Display, TEXT("KayKit: Dialogue %s: %s (%d choices)"), *Speaker,*DialogueText,Choices.Num());
}
void AKayKitController::ExpandChoice(const FArcweaveConnectionsData& Connection, const FString& InLabel, int32 Depth)
{
    if (Depth>32) return;
    FString Label=InLabel;
    const auto Board=FindBoardFor(Connection.Sourceid);
    if (Label.IsEmpty()) Label=HasScript(Connection.Label) ? Arcweave->TranspileConnectionLabel(Connection,Board) : PlainText(Connection.Label);
    if (Connection.TargetType==TEXT("branches"))
    {
        const auto TargetBoard=FindBoardFor(Connection.Targetid);
        const auto Branch=Arcweave->GetIsTargetBranch(TargetBoard,Connection);
        for (const auto& Next : Branch.BranchConnections) ExpandChoice(Next,Label,Depth+1);
        return;
    }
    FKayKitDialogueChoice Choice;
    Choice.Label=Label.IsEmpty() ? TEXT("Next") : PlainText(Label);
    Choice.Connection=Connection;
    Choices.Add(Choice);
}
void AKayKitController::Choose(int32 Index)
{
    if (!bInDialogue) return;
    if (bEndElement) { EndDialogue(); return; }
    if (Choices.IsValidIndex(Index)) { const auto Connection=Choices[Index].Connection; FollowConnection(Connection); }
    else if (Choices.IsEmpty()) EndDialogue();
}
void AKayKitController::FollowConnection(const FArcweaveConnectionsData& Connection, int32 Depth)
{
    if (Depth>48) { EndDialogue(); return; }
    if (HasScript(Connection.Label)) Arcweave->UpdateVariablesFromConnection(Connection);
    if (Connection.TargetType==TEXT("jumpers"))
    {
        const auto Board=FindBoardFor(Connection.Targetid);
        for (const auto& Jumper : Board.Jumpers) if (Jumper.Id==Connection.Targetid) { EnterElement(Jumper.ElementData.Id,Depth+1); return; }
    }
    else if (Connection.TargetType==TEXT("branches"))
    {
        const auto Result=Arcweave->GetIsTargetBranch(FindBoardFor(Connection.Targetid),Connection);
        if (Result.BranchConnections.Num()) FollowConnection(Result.BranchConnections[0],Depth+1);
        else EndDialogue();
    }
    else EnterElement(Connection.Targetid,Depth+1);
}
