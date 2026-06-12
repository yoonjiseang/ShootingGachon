#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootingEnemy.generated.h"

class AShootingBullet;
class AShootingPlayer;
class AShootingPowerUpOrb;
class AShootingWaveManager;
class UNiagaraSystem;
class UBoxComponent;
class UPrimitiveComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingEnemy : public APawn
{
	GENERATED_BODY()

public:
	AShootingEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float HP = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 ScoreValue = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool Dead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<AShootingWaveManager> WaveManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<AShootingPlayer> PlayerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float EscapeZ = -600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float HorizontalDriftAmplitude = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float HorizontalDriftSpeed = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PlayerChaseWeight = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	TSubclassOf<AShootingPowerUpOrb> PowerUpOrbClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PowerUpDropChance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> DeathSound;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamageToEnemy(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void Die(int32 ScoreToGive);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void TryDropPowerUp();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void HandleOverlap(AActor* OtherActor);

protected:
	UFUNCTION()
	void OnCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void PlayDeathFeedback();
	void RemoveEscapedEnemy();

	float SpawnDepthX = 0.0f;
	float SpawnBaseY = 0.0f;
	float DriftPhase = 0.0f;
};
