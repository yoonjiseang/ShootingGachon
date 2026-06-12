#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootingBossEnemy.generated.h"

class AShootingBossBullet;
class AShootingWaveManager;
class APawn;
class UNiagaraSystem;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingBossEnemy : public APawn
{
	GENERATED_BODY()

public:
	AShootingBossEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UStaticMeshComponent> BossMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<USceneComponent> BeamStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TObjectPtr<AShootingWaveManager> WaveManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TObjectPtr<APawn> PlayerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TSubclassOf<AShootingBossBullet> BossBulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HP = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ScoreValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool Dead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DesiredDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DistanceTolerance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float BeamTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float BeamCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float BeamDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> DeathSound;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ApplyDamageToEnemy(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void Die(int32 ScoreToGive);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void FireBossBullet();

	UFUNCTION(BlueprintCallable, Category = "Boss")
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

	void UpdateDistanceMovement(float DeltaSeconds);
	void UpdateFireTimer(float DeltaSeconds);
	void PlayDeathFeedback();
};
