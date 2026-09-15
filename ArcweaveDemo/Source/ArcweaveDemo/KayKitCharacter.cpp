#include "KayKitCharacter.h"
#include "KayKitController.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AKayKitCharacter::AKayKitCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(28, 90);
    GetMesh()->SetRelativeLocation(FVector(0,0,-90));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->MaxWalkSpeed = 500;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);
    bUseControllerRotationYaw = false;
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetRelativeLocation(FVector(0,0,-90));
    CameraBoom->TargetArmLength = 800;
    CameraBoom->SocketOffset = FVector(0,0,200);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10;
    CameraBoom->bDoCollisionTest = false;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->FieldOfView = 91.49f; // Unity's 60-degree vertical FOV at 16:9.
}
void AKayKitCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!DialogueBoard.IsEmpty()) { FollowCamera->Deactivate(); GetCharacterMovement()->DisableMovement(); }
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
}
void AKayKitCharacter::Attack(float Speed)
{
    if (AttackAnimation && GetWorld()->GetTimeSeconds() >= AttackUntil)
    {
        PlayingAnimation = AttackAnimation;
        GetMesh()->PlayAnimation(AttackAnimation, false);
        GetMesh()->GlobalAnimRateScale = FMath::Max(Speed, .1f);
        AttackUntil = GetWorld()->GetTimeSeconds() + AttackAnimation->GetPlayLength() / GetMesh()->GlobalAnimRateScale;
    }
}
void AKayKitCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* PC = Cast<AKayKitController>(UGameplayStatics::GetPlayerController(this, 0));
    if (GetWorld()->GetTimeSeconds() < AttackUntil) return;
    UAnimSequence* Desired = GetVelocity().SizeSquared2D() > 25 ? WalkAnimation.Get() : IdleAnimation.Get();
    if (!HealthVariable.IsEmpty() && PC && PC->VariableNumber(HealthVariable, 100) < 40 && InjuredAnimation) Desired = InjuredAnimation;
    if (Desired && Desired != PlayingAnimation)
    {
        PlayingAnimation = Desired;
        GetMesh()->GlobalAnimRateScale = 1;
        GetMesh()->PlayAnimation(Desired, true);
    }
}
