#include "KayKitGameMode.h"
#include "KayKitController.h"
#include "KayKitHUD.h"

AKayKitGameMode::AKayKitGameMode()
{
    DefaultPawnClass=nullptr;
    PlayerControllerClass=AKayKitController::StaticClass();
    HUDClass=AKayKitHUD::StaticClass();
}
