#include "ShootingGameMode.h"

#include "GameFramework/PlayerController.h"

AShootingGameMode::AShootingGameMode()
{
	// The shooting map already has its player pawn placed in the level.
	// Keeping this null prevents the default ThirdPerson pawn from stealing focus.
	DefaultPawnClass = nullptr;
	PlayerControllerClass = APlayerController::StaticClass();
}
