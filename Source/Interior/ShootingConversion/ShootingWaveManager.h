#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingWaveManager.generated.h"

class AShootingBossEnemy;
class AShootingEnemy;
class UUserWidget;

UCLASS(Blueprintable)
class INTERIOR_API AShootingWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AShootingWaveManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AActor> SpawnPointClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AShootingEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AShootingBossEnemy> BossEnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TObjectPtr<AActor> BossSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	FName RestartLevelName = TEXT("ShootingMap_Wave");

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	TObjectPtr<UUserWidget> HUDRef;

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	TObjectPtr<AShootingBossEnemy> CurrentBossRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 CurrentWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 EnemiesToSpawnThisWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 SpawnedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 AliveEnemyCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool WaveRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	int32 BossWaveInterval = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool IsBossWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool BossSpawnedThisWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool ShowBossUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float BossHPPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float PlayerHP = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float MaxPlayerHP = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	bool PlayerInvincible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float InvincibleRemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float DamageBuffRemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText StartMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShowGameOverUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool GameOver = false;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SpawnOneEnemy();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SpawnBossEnemy();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyEnemyRemoved(int32 ScoreAmount);

	UFUNCTION(BlueprintCallable, Category = "Player")
	void DamagePlayer(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void GameOverEvent();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHUD();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateBossUI(bool bShowUI, float NewPercent);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void UpdatePowerUpTimers(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void ApplyHealPowerUp(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void ApplyInvinciblePowerUp(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void ApplyDamagePowerUp(float Multiplier, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartGame();

protected:
	FTimerHandle ClearStartMessageTimerHandle;
	FTimerHandle NextWaveTimerHandle;

	void ResetRuntimeStateForNewGame();
	void ClearStartMessageAndRefresh();
	void ScheduleNextWave();
};
