#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KayKitCharacter.generated.h"

class UAnimSequence;
class UCameraComponent;
class USpringArmComponent;
class USoundBase;

/** Shared KayKit character used by the player and the three narrative NPCs. */
UCLASS()
class ARCWEAVEDEMO_API AKayKitCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AKayKitCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") FString CharacterName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") FString DialogueBoard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") FString HealthVariable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") float InteractionDistance = 300;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") TObjectPtr<UAnimSequence> InjuredAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") TObjectPtr<UAnimSequence> AttackAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") TMap<FString, TObjectPtr<USoundBase>> DialogueSounds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Example") FLinearColor HealthColor = FLinearColor::Green;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> FollowCamera;
    UFUNCTION(BlueprintCallable, Category="Example") void Attack(float Speed = 1);
private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Example", meta=(AllowPrivateAccess="true")) TObjectPtr<UAnimSequence> PlayingAnimation;
    float AttackUntil = 0;
};
