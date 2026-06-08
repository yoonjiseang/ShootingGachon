#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootingPlayer.generated.h"

class AShootingBullet;
class AShootingWaveManager;
class USceneComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingPlayer : public APawn
{
	GENERATED_BODY()

public:
	AShootingPlayer();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooting")
	TObjectPtr<USceneComponent> FirePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	TSubclassOf<AShootingBullet> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	TObjectPtr<AShootingWaveManager> WaveManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float BaseBulletDamage = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void FireBullet();
};
