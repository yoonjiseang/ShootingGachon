#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootingPlayer.generated.h"

class AShootingBullet;
class AShootingWaveManager;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingPlayer : public APawn
{
	GENERATED_BODY()

public:
	AShootingPlayer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UStaticMeshComponent> ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooting")
	TObjectPtr<USceneComponent> FirePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	TSubclassOf<AShootingBullet> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	TObjectPtr<AShootingWaveManager> WaveManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float BaseBulletDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinY = -520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxY = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinZ = -650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxZ = 1050.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float FireCooldown = 0.15f;

	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void FireBullet();

protected:
	UFUNCTION()
	void MoveHorizontal(float Value);

	UFUNCTION()
	void MoveVertical(float Value);

	float HorizontalInput = 0.0f;
	float VerticalInput = 0.0f;
	float FireTimer = 0.0f;
	float PlayfieldDepthX = 0.0f;
};
