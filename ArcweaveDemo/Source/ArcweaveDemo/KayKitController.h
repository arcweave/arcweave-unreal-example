#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArcweaveProjectData.h"
#include "KayKitController.generated.h"

class AKayKitCharacter;
class UArcweaveSubsystem;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class SWidget;

USTRUCT(BlueprintType)
struct FKayKitDialogueChoice
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString Label;
    UPROPERTY(BlueprintReadOnly) FArcweaveConnectionsData Connection;
};

/** Engine-specific interaction adapter. Story evaluation remains in the Arcweave plugin. */
UCLASS()
class ARCWEAVEDEMO_API AKayKitController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UFUNCTION(BlueprintCallable, Category="Example") bool StartDialogue(AKayKitCharacter* NPC);
    UFUNCTION(BlueprintCallable, Category="Example") void Choose(int32 Index);
    UFUNCTION(BlueprintCallable, Category="Example") void EndDialogue();
    UFUNCTION(BlueprintCallable, Category="Example") float VariableNumber(const FString& Name, float Fallback = 0) const;
    UFUNCTION(BlueprintCallable, Category="Example") FString VariableValue(const FString& Name) const;
    UFUNCTION(BlueprintPure, Category="Example") TArray<FString> VariableNames() const;
    UFUNCTION(BlueprintPure, Category="Example") bool HasVariable(const FString& Name) const;
    const TArray<FString>& GetVariableDisplayLines() const { return VariableDisplayLines; }
    uint32 GetVariableDisplayRevision() const { return VariableDisplayRevision; }
    UFUNCTION(BlueprintCallable, Category="Example") void ToggleMenu();
    UFUNCTION(BlueprintCallable, Category="Example") void LoadStory(bool Local, const FString& Token = TEXT(""), const FString& Hash = TEXT(""));
    UPROPERTY(BlueprintReadOnly) bool bInDialogue = false;
    UPROPERTY(BlueprintReadOnly) bool bMenuOpen = false;
    UPROPERTY(BlueprintReadOnly) bool bStoryReady = false;
    UPROPERTY(BlueprintReadOnly) bool bCanAttack = false;
    UPROPERTY(BlueprintReadOnly) FString DialogueText;
    UPROPERTY(BlueprintReadOnly) FString Speaker;
    UPROPERTY(BlueprintReadOnly) FString PortraitName;
    UPROPERTY(BlueprintReadOnly) FString CurrentElementId;
    UPROPERTY(BlueprintReadOnly) FString StatusMessage;
    UPROPERTY(BlueprintReadOnly) TArray<FKayKitDialogueChoice> Choices;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<AKayKitCharacter> NearbyNPC;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<AKayKitCharacter> ActiveNPC;
    static FString PlainText(FString Text);
private:
    friend class FKayKitProjectVariablesCommand;
    UPROPERTY() TObjectPtr<UArcweaveSubsystem> Arcweave;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Rain;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> BackgroundMaterial;
    TMap<FString, FArcweaveVariable> RuntimeVariables;
    TMap<FString, FString> VariableIdsByName;
    TArray<FString> SortedVariableNames;
    TArray<FString> VariableDisplayLines;
    uint32 VariableDisplayRevision = 0;
    TArray<FArcweaveComponentData> SceneComponents;
    TMap<TWeakObjectPtr<AKayKitCharacter>, FLinearColor> DefaultHealthColors;
    UFUNCTION() void OnProjectLoaded(const FArcweaveProjectData& Data);
    UFUNCTION() void OnVariablesChanged(const TArray<FArcweaveVariable>& Variables);
    UFUNCTION() void OnFetchCompleted(bool bSuccess, const FString& Message);
    void RebuildVariableDisplay();
    void ApplySceneAttributes();
    FString AttributeValue(const FArcweaveAttributeData& Attribute) const;
    void EnterElement(const FString& Id, int32 Depth = 0);
    void FollowConnection(const FArcweaveConnectionsData& Connection, int32 Depth = 0);
    void ExpandChoice(const FArcweaveConnectionsData& Connection, const FString& Label, int32 Depth = 0);
    FArcweaveBoardData FindBoardFor(const FString& Id) const;
    void SetUIInput(bool Enabled);
    TSharedPtr<SWidget> MenuWidget;
    FString SessionAPIToken;
    FString SessionProjectHash;
    bool bImportRequested = false;
    bool bStoryReadyBeforeImport = false;
    float SwingSpeed = 1;
    float NPCScanTime = 0;
    bool bEndElement = false;
    bool bPotionCollected = false;
    bool bRainEnabled = false;
};
