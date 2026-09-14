#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "KayKitController.h"
#include "KayKitCharacter.h"
#include "KayKitHUD.h"
#include "ArcweaveSubsystem.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Engine/Engine.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerInput.h"
#include "InputKeyEventArgs.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKayKitControlsTest, "Arcweave.KayKit.Controls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FKayKitControlsCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    TWeakObjectPtr<AKayKitController> PC;
    int32 Step=0;
    double Next=0;
    FVector Start;
    double Deadline=FPlatformTime::Seconds()+30;
public:
    FKayKitControlsCommand(FAutomationTestBase* InTest):Test(InTest){}
    virtual bool Update() override
    {
        if (!PC.IsValid())
        {
            for (const auto& Context : GEngine->GetWorldContexts())
                if (Context.WorldType==EWorldType::PIE)
                    for (TActorIterator<AKayKitController> It(Context.World());It;++It) PC=*It;
            if (!PC.IsValid())
            {
                if (FPlatformTime::Seconds()>Deadline) { Test->AddError(TEXT("KayKit Play session did not start.")); return true; }
                return false;
            }
        }
        if (FPlatformTime::Seconds()<Next) return false;
        auto* P=PC.Get();
        auto* Pawn=Cast<AKayKitCharacter>(P->GetPawn());
        auto Key=[&](FKey K,EInputEvent E){P->InputKey(FInputKeyEventArgs(nullptr,FInputDeviceId::CreateFromInternalId(0),K,E,FPlatformTime::Cycles64()));};
        auto Click=[&]()
        {
            int32 W,H; P->GetViewportSize(W,H);
            const float S=FMath::Min(W/1280.f,H/720.f);
            P->SetMouseLocation(FMath::RoundToInt(850*S),FMath::RoundToInt(H-73*S));
            Key(EKeys::LeftMouseButton,IE_Pressed); Key(EKeys::LeftMouseButton,IE_Released);
        };
        switch(Step++)
        {
        case 0:
            P->EndDialogue();
            GEngine->GetEngineSubsystem<UArcweaveSubsystem>()->FetchData(TEXT(""),TEXT(""));
            P->SetControlRotation(FRotator::ZeroRotator);
            Pawn->SetActorLocation(FVector(161,358,98),false);
            Start=Pawn->GetActorLocation();
            Key(EKeys::W,IE_Pressed);
            Next=FPlatformTime::Seconds()+1.8;
            return false;
        case 1:
            Key(EKeys::W,IE_Released);
            Test->TestTrue(TEXT("W moves the player"),Pawn->GetActorLocation().X>Start.X+200);
            Test->TestTrue(TEXT("Dungeon wall stops the player"),Pawn->GetActorLocation().X<780);
            Pawn->GetCharacterMovement()->StopMovementImmediately();
            Pawn->SetActorLocation(FVector(-407,280,98),false);
            break;
        case 2:
            Test->TestTrue(TEXT("Nearby healer offers interaction"),P->NearbyNPC && P->NearbyNPC->CharacterName==TEXT("Healer"));
            Key(EKeys::E,IE_Pressed);
            break;
        case 3:
            Key(EKeys::E,IE_Released);
            Test->TestTrue(TEXT("E opens dialogue"),P->bInDialogue && P->DialogueText.Contains(TEXT("Hello traveler")));
            Click();
            break;
        case 4:
            Test->TestTrue(TEXT("Mouse selects Ask for help"),P->DialogueText.Contains(TEXT("grave danger")));
            Click();
            break;
        case 5:
            Test->TestEqual(TEXT("Potion is granted"),P->VariableValue(TEXT("have_potion")),FString(TEXT("true")));
            Test->TestTrue(TEXT("Final line stays visible"),P->bInDialogue && P->DialogueText.Contains(TEXT("healing potion")));
            Click();
            break;
        case 6:
            Test->TestFalse(TEXT("Final choice closes dialogue"),P->bInDialogue);
            P->EndDialogue();
            GEngine->GetEngineSubsystem<UArcweaveSubsystem>()->FetchData(TEXT(""),TEXT(""));
            Pawn->SetActorLocation(FVector(161,358,98),false);
            P->SetControlRotation(FRotator::ZeroRotator);
            return true;
        }
        Next=FPlatformTime::Seconds()+.3;
        return false;
    }
};

bool FKayKitControlsTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/KayKit/Maps/ArcweaveDemo")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FKayKitControlsCommand(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKayKitProjectVariablesTest, "Arcweave.KayKit.ProjectVariables",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FKayKitProjectVariablesCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    double Deadline=FPlatformTime::Seconds()+30;
public:
    FKayKitProjectVariablesCommand(FAutomationTestBase* InTest):Test(InTest){}
    virtual bool Update() override
    {
        AKayKitController* PC=nullptr;
        for (const auto& Context : GEngine->GetWorldContexts())
            if (Context.WorldType==EWorldType::PIE)
                for (TActorIterator<AKayKitController> It(Context.World());It;++It) PC=*It;
        if (!PC)
        {
            if (FPlatformTime::Seconds()>Deadline) { Test->AddError(TEXT("KayKit Play session did not start.")); return true; }
            return false;
        }

        auto* Arcweave=GEngine->GetEngineSubsystem<UArcweaveSubsystem>();
        FArcweaveProjectData Project;
        Project.Name=TEXT("Variable display test");
        Project.Boards.AddDefaulted();
        const auto AddVariable=[&](const TCHAR* Id,const TCHAR* Name,const TCHAR* Scope,const TCHAR* Value)
        {
            FArcweaveVariable Variable;
            Variable.Id=Id; Variable.Name=Name; Variable.Scope=Scope; Variable.Value=Value;
            Project.CurrentVars.Add(Id,Variable);
        };
        AddVariable(TEXT("global-health"),TEXT("health"),TEXT(""),TEXT("100"));
        AddVariable(TEXT("board-health"),TEXT("health"),TEXT("castle"),TEXT("10"));
        AddVariable(TEXT("component-health"),TEXT("health"),TEXT("hero"),TEXT("20"));
        AddVariable(TEXT("component-label"),TEXT("label"),TEXT("hero"),TEXT(""));
        Arcweave->OnArcweaveResponseReceived.Broadcast(Project);

        Test->TestEqual(TEXT("HUD lists only imported variables with their scopes"),
            FString::Join(PC->VariableNames(),TEXT(",")),FString(TEXT("castle.health,health,hero.health,hero.label")));
        Test->TestEqual(TEXT("Global lookup does not select a scoped namesake"),PC->VariableValue(TEXT("health")),FString(TEXT("100")));
        Test->TestEqual(TEXT("Board lookup resolves its scope"),PC->VariableValue(TEXT("castle.health")),FString(TEXT("10")));
        Test->TestEqual(TEXT("Component lookup resolves its scope"),PC->VariableValue(TEXT("hero.health")),FString(TEXT("20")));
        Test->TestTrue(TEXT("Empty-string variables remain visible"),PC->VariableNames().Contains(TEXT("hero.label")));
        Test->TestFalse(TEXT("Bundled gold is not retained after a different import"),PC->VariableNames().Contains(TEXT("gold")));

        FArcweaveVariable Changed=Project.CurrentVars[TEXT("component-health")];
        Changed.Value=TEXT("21");
        Arcweave->OnArcweaveVariableChanged.Broadcast(TArray<FArcweaveVariable>{Changed});
        Test->TestEqual(TEXT("Variable events update the displayed component value"),PC->VariableValue(TEXT("hero.health")),FString(TEXT("21")));
        Test->TestEqual(TEXT("Component changes preserve the global value"),PC->VariableValue(TEXT("health")),FString(TEXT("100")));
        Test->TestEqual(TEXT("Component changes preserve the board value"),PC->VariableValue(TEXT("castle.health")),FString(TEXT("10")));

        Project.CurrentVars.Reset();
        Arcweave->OnArcweaveResponseReceived.Broadcast(Project);
        Test->TestTrue(TEXT("An empty project removes all previous variable rows"),PC->VariableNames().IsEmpty());
        PC->LoadStory(true);
        Test->TestTrue(TEXT("Bundled reload restores its global variables"),PC->VariableNames().Contains(TEXT("gold")));
        Test->TestTrue(TEXT("Bundled reload includes eligible component attributes"),PC->VariableNames().Contains(TEXT("healer.color")));
        Test->TestFalse(TEXT("Bundled reload removes the previous project's scoped variables"),PC->VariableNames().Contains(TEXT("hero.health")));

        const auto Bundled = Arcweave->GetArcweaveProjectData();
        const auto SetScoped = [&](const TCHAR* Scope, const TCHAR* Name, const TCHAR* Value)
        {
            for (const auto& Pair : Bundled.CurrentVars)
                if (Pair.Value.Scope == Scope && Pair.Value.Name == Name)
                    Arcweave->SetVariable(Pair.Key, Value);
        };
        Test->TestTrue(TEXT("Bundled rain is initially visible"), PC->Rain->IsVisible());
        SetScoped(TEXT("environment"), TEXT("weather"), TEXT("Clear"));
        SetScoped(TEXT("environment"), TEXT("time"), TEXT("Night"));
        SetScoped(TEXT("healer"), TEXT("color"), TEXT("Blue"));
        Test->TestFalse(TEXT("Changing the weather attribute hides rain immediately"), PC->Rain->IsVisible());
        FLinearColor Sky;
        PC->BackgroundMaterial->GetVectorParameterValue(TEXT("SkyColor"), Sky);
        Test->TestTrue(TEXT("Changing the time attribute updates the sky immediately"), Sky.Equals(FLinearColor::Black));
        for (TActorIterator<AKayKitCharacter> It(PC->GetWorld()); It; ++It)
            if (It->CharacterName == TEXT("Healer"))
                Test->TestTrue(TEXT("Changing the color attribute updates the health bar immediately"), It->HealthColor.Equals(FLinearColor(.15f,.35f,1.f)));
        const uint32 Revision = PC->GetVariableDisplayRevision();
        PC->VariableNames(); PC->VariableValue(TEXT("healer.color"));
        Test->TestEqual(TEXT("Reading variables reuses the display cache"), PC->GetVariableDisplayRevision(), Revision);

        PC->bStoryReadyBeforeImport = PC->bStoryReady;
        PC->bStoryReady = false;
        PC->bImportRequested = true;
        Arcweave->OnArcweaveFetchCompleted.Broadcast(false, TEXT("Test import failed. Please retry."));
        Test->TestTrue(TEXT("A failed import restores the previous playable story"), PC->bStoryReady);
        Test->TestTrue(TEXT("A failed import opens the menu with its error"), PC->bMenuOpen && PC->StatusMessage.Contains(TEXT("Test import failed")));
        Test->TestEqual(TEXT("A failed import retains the previous variable values"), PC->VariableValue(TEXT("environment.time")), FString(TEXT("Night")));
        PC->ToggleMenu();
        Arcweave->OnArcweaveResponseReceived.Broadcast(Project);
        Test->TestFalse(TEXT("A project without weather resets rain"), PC->Rain->IsVisible());
        PC->BackgroundMaterial->GetVectorParameterValue(TEXT("SkyColor"), Sky);
        Test->TestTrue(TEXT("A project without time resets the sky"), Sky.Equals(FLinearColor(.24f,.30f,.33f)));
        PC->LoadStory(true);
        return true;
    }
};

bool FKayKitProjectVariablesTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/KayKit/Maps/ArcweaveDemo")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FKayKitProjectVariablesCommand(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKayKitVariablePanelTest, "Arcweave.KayKit.VariablePanel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FKayKitVariablePanelCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    int32 Phase = 0;
    double Next = 0;
    double Deadline = FPlatformTime::Seconds() + 30;
public:
    FKayKitVariablePanelCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        AKayKitController* PC = nullptr;
        for (const auto& Context : GEngine->GetWorldContexts())
            if (Context.WorldType == EWorldType::PIE)
                for (TActorIterator<AKayKitController> It(Context.World()); It; ++It) PC = *It;
        if (!PC)
        {
            if (FPlatformTime::Seconds() > Deadline) { Test->AddError(TEXT("PIE did not start.")); return true; }
            return false;
        }
        if (FPlatformTime::Seconds() < Next) return false;
        auto* HUD = Cast<AKayKitHUD>(PC->GetHUD());
        if (!HUD) { Test->AddError(TEXT("Variable HUD is missing.")); return true; }
        if (Phase == 0)
        {
            FArcweaveProjectData Project;
            Project.Boards.AddDefaulted();
            for (int32 Index = 0; Index < 80; ++Index)
            {
                FArcweaveVariable Variable;
                Variable.Id = Variable.Name = FString::Printf(TEXT("variable_%03d"), Index);
                Variable.Value = FString::ChrN(250, TCHAR('W'));
                Project.CurrentVars.Add(Variable.Id, Variable);
            }
            GEngine->GetEngineSubsystem<UArcweaveSubsystem>()->OnArcweaveResponseReceived.Broadcast(Project);
        }
        else if (Phase == 1)
        {
            Test->TestTrue(TEXT("Large projects have multiple variable pages"), HUD->VariablePageCount > 1);
            Test->TestTrue(TEXT("Variable panel fits within the viewport"), HUD->VariablePanelWidth < HUD->CachedViewportSize.X / 2);
            Test->TestTrue(TEXT("Long variable values are ellipsized"), HUD->ClippedVariableLines[0].EndsWith(TEXT("...")));
            PC->InputKey(FInputKeyEventArgs(nullptr, FInputDeviceId::CreateFromInternalId(0), EKeys::PageDown, IE_Pressed, FPlatformTime::Cycles64()));
        }
        else
        {
            PC->InputKey(FInputKeyEventArgs(nullptr, FInputDeviceId::CreateFromInternalId(0), EKeys::PageDown, IE_Released, FPlatformTime::Cycles64()));
            Test->TestEqual(TEXT("Page Down advances the variable panel"), HUD->VariablePage, 1);
            PC->LoadStory(true);
            return true;
        }
        ++Phase;
        Next = FPlatformTime::Seconds() + .3;
        return false;
    }
};

bool FKayKitVariablePanelTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/KayKit/Maps/ArcweaveDemo")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FKayKitVariablePanelCommand(this));
    return true;
}
#endif
