#include "KayKitController.h"
#include "KayKitCharacter.h"
#include "KayKitHUD.h"
#include "ArcweaveSubsystem.h"
#include "ArcweaveAPISettings.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"

void AKayKitController::BeginPlay()
{
    Super::BeginPlay();
    Arcweave = GEngine->GetEngineSubsystem<UArcweaveSubsystem>();
    Arcweave->OnArcweaveResponseReceived.AddDynamic(this, &AKayKitController::OnProjectLoaded);
    Arcweave->OnArcweaveVariableChanged.AddDynamic(this, &AKayKitController::OnVariablesChanged);
    Arcweave->OnArcweaveFetchCompleted.AddDynamic(this, &AKayKitController::OnFetchCompleted);
    for (TActorIterator<AKayKitCharacter> It(GetWorld()); It; ++It)
        DefaultHealthColors.Add(*It, It->HealthColor);
    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
        if (It->ActorHasTag(TEXT("ArcweaveBackground")))
            BackgroundMaterial = It->GetStaticMeshComponent()->CreateDynamicMaterialInstance(0);
    SetControlRotation(FRotator(0,0,0));
    bEnableClickEvents=true;
    bEnableMouseOverEvents=true;
    SetUIInput(false);
    Rain=NewObject<UInstancedStaticMeshComponent>(this,TEXT("Rain"));
    Rain->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Rain->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/KayKit/Materials/M_Rain.M_Rain")));
    Rain->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Rain->SetCastShadow(false);
    Rain->SetMobility(EComponentMobility::Movable);
    Rain->RegisterComponent();
    SetActorHiddenInGame(false);
    for (int32 I=0;I<160;++I) Rain->AddInstance(FTransform(FRotator::ZeroRotator,FVector::ZeroVector,FVector(.008,.008,.35)),true);
    const auto Settings = Arcweave->LoadArcweaveSettings();
    LoadStory(Settings.EnableReceiveMethodFromLocalJSON, Settings.APIToken, Settings.Hash);
}
void AKayKitController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Arcweave)
    {
        Arcweave->CancelFetch();
        Arcweave->OnArcweaveResponseReceived.RemoveDynamic(this, &AKayKitController::OnProjectLoaded);
        Arcweave->OnArcweaveVariableChanged.RemoveDynamic(this, &AKayKitController::OnVariablesChanged);
        Arcweave->OnArcweaveFetchCompleted.RemoveDynamic(this, &AKayKitController::OnFetchCompleted);
    }
    if (MenuWidget && GEngine->GameViewport) GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
    Super::EndPlay(Reason);
}
void AKayKitController::SetUIInput(bool Enabled)
{
    bShowMouseCursor = Enabled;
    if (Enabled) SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
    else SetInputMode(FInputModeGameOnly());
}
void AKayKitController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Rain && bRainEnabled)
    {
        const float Time=GetWorld()->GetTimeSeconds();
        for (int32 I=0;I<Rain->GetInstanceCount();++I)
        {
            const FVector Location(-1100+FMath::Fmod(I*683.f,2000.f),-1000+FMath::Fmod(I*457.f,1900.f),1200-FMath::Fmod(Time*650+I*97.f,1200.f));
            Rain->UpdateInstanceTransform(I,FTransform(FRotator::ZeroRotator,Location,FVector(.008,.008,.35)),true,false,true);
        }
        Rain->MarkRenderStateDirty();
    }
    if (WasInputKeyJustPressed(EKeys::Escape) && !bInDialogue) ToggleMenu();
    if (!bMenuOpen)
        if (auto* HUD = Cast<AKayKitHUD>(GetHUD()))
        {
            if (WasInputKeyJustPressed(EKeys::PageDown)) HUD->ChangeVariablePage(1);
            if (WasInputKeyJustPressed(EKeys::PageUp)) HUD->ChangeVariablePage(-1);
        }
    auto* ControlledPawn = Cast<AKayKitCharacter>(GetPawn());
    if (!ControlledPawn || bInDialogue || bMenuOpen) return;
    float DX=0, DY=0;
    GetInputMouseDelta(DX, DY);
    FRotator Rotation=GetControlRotation();
    Rotation.Yaw += DX * .3f;
    Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch + DY*.3f), -45.f, 0.f);
    SetControlRotation(Rotation);
    const FRotationMatrix Direction(FRotator(0,Rotation.Yaw,0));
    FVector Movement = Direction.GetUnitAxis(EAxis::X) * (float(IsInputKeyDown(EKeys::W)||IsInputKeyDown(EKeys::Up))-float(IsInputKeyDown(EKeys::S)||IsInputKeyDown(EKeys::Down)))
        + Direction.GetUnitAxis(EAxis::Y) * (float(IsInputKeyDown(EKeys::D)||IsInputKeyDown(EKeys::Right))-float(IsInputKeyDown(EKeys::A)||IsInputKeyDown(EKeys::Left)));
    ControlledPawn->AddMovementInput(Movement.GetSafeNormal());
    NPCScanTime -= DeltaSeconds;
    if (NPCScanTime <= 0)
    {
        NPCScanTime=.1f;
        NearbyNPC=nullptr;
        float Best=TNumericLimits<float>::Max();
        for (TActorIterator<AKayKitCharacter> It(GetWorld()); It; ++It)
        {
            if (It->DialogueBoard.IsEmpty()) continue;
            const float Distance=FVector::DistSquared(ControlledPawn->GetActorLocation(), It->GetActorLocation());
            if (Distance<FMath::Square(It->InteractionDistance) && Distance<Best) { NearbyNPC=*It; Best=Distance; }
        }
    }
    if (WasInputKeyJustPressed(EKeys::E) && NearbyNPC) StartDialogue(NearbyNPC);
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton) && bCanAttack) ControlledPawn->Attack(SwingSpeed);
}
