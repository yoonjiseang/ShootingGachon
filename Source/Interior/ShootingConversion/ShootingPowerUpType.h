#pragma once

#include "CoreMinimal.h"
#include "ShootingPowerUpType.generated.h"

UENUM(BlueprintType)
enum class EShootingPowerUpType : uint8
{
	DamageUp UMETA(DisplayName = "DamageUp"),
	Invincible UMETA(DisplayName = "Invincible"),
	Heal UMETA(DisplayName = "Heal")
};
