#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "KayKitHUD.generated.h"

class AKayKitController;

UCLASS()
class ARCWEAVEDEMO_API AKayKitHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;
    void ChangeVariablePage(int32 Direction);
private:
    friend class FKayKitVariablePanelCommand;
    UPROPERTY() TMap<FString, TObjectPtr<UTexture2D>> Portraits;
    void DrawLabel(const FString& Text, float X, float Y, float Size, FLinearColor Color = FLinearColor::White);
    FVector2D MeasureLabel(const FString& Text, float Size) const;
    void DrawVariables(AKayKitController* Controller, float Scale);
    TArray<FString> ClippedVariableLines;
    uint32 CachedVariableRevision = MAX_uint32;
    FVector2D CachedViewportSize = FVector2D::ZeroVector;
    int32 VariablePage = 0;
    int32 VariablePageCount = 1;
    float VariablePanelWidth = 0;
};
