#include "ShootingWaveManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ShootingBossEnemy.h"
#include "ShootingEnemy.h"
#include "ShootingGameHUDWidget.h"
#include "TimerManager.h"

AShootingWaveManager::AShootingWaveManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShootingWaveManager::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnPointClass)
	{
		TArray<AActor*> FoundSpawnPoints;
		UGameplayStatics::GetAllActorsOfClass(this, SpawnPointClass, FoundSpawnPoints);
		SpawnPoints.Reset();
		for (AActor* SpawnPoint : FoundSpawnPoints)
		{
			SpawnPoints.Add(SpawnPoint);
		}
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (HUDWidgetClass && PlayerController)
	{
		HUDRef = CreateWidget<UShootingGameHUDWidget>(PlayerController, HUDWidgetClass);
		if (HUDRef)
		{
			HUDRef->AddToViewport();
		}
	}

	RefreshHUD();
	StartWave();

	EnableInput(PlayerController);
	if (InputComponent)
	{
		FInputKeyBinding& RestartBinding = InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AShootingWaveManager::RestartGame);
		RestartBinding.bExecuteWhenPaused = true;
	}
}

void AShootingWaveManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (InvincibleRemainingTime > 0.0f || DamageBuffRemainingTime > 0.0f)
	{
		UpdatePowerUpTimers(DeltaSeconds);
	}

	if (!WaveRunning || SpawnedCount >= EnemiesToSpawnThisWave)
	{
		return;
	}

	SpawnTimer += DeltaSeconds;
	if (SpawnTimer >= SpawnCooldown)
	{
		SpawnTimer = 0.0f;
		SpawnOneEnemy();
	}
}

void AShootingWaveManager::StartWave()
{
	if (GameOver)
	{
		return;
	}

	CurrentWave += 1;
	IsBossWave = BossWaveInterval > 0 && CurrentWave % BossWaveInterval == 0;
	BossSpawnedThisWave = false;
	ShowBossUI = false;
	BossHPPercent = 1.0f;

	EnemiesToSpawnThisWave = CurrentWave * 2 + 2;
	SpawnedCount = 0;
	AliveEnemyCount = 0;
	SpawnTimer = 0.0f;
	ShowGameOverUI = false;
	WaveRunning = true;

	StartMessage = FText::Format(FText::FromString(TEXT("WAVE {0} START")), FText::AsNumber(CurrentWave));
	RefreshHUD();

	if (IsBossWave)
	{
		SpawnBossEnemy();
	}

	GetWorldTimerManager().ClearTimer(ClearStartMessageTimerHandle);
	GetWorldTimerManager().SetTimer(
		ClearStartMessageTimerHandle,
		this,
		&AShootingWaveManager::ClearStartMessageAndRefresh,
		1.2f,
		false);
}

void AShootingWaveManager::SpawnOneEnemy()
{
	if (!EnemyClass || SpawnPoints.Num() <= 0)
	{
		return;
	}

	const int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	AActor* SpawnPoint = SpawnPoints[SpawnIndex];
	if (!IsValid(SpawnPoint))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShootingEnemy* Enemy = GetWorld()->SpawnActor<AShootingEnemy>(EnemyClass, SpawnPoint->GetActorTransform(), SpawnParams);
	if (!IsValid(Enemy))
	{
		return;
	}

	Enemy->WaveManagerRef = this;
	SpawnedCount += 1;
	AliveEnemyCount += 1;
}

void AShootingWaveManager::SpawnBossEnemy()
{
	if (BossSpawnedThisWave || !BossEnemyClass || !IsValid(BossSpawnPoint))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShootingBossEnemy* Boss = GetWorld()->SpawnActor<AShootingBossEnemy>(BossEnemyClass, BossSpawnPoint->GetActorTransform(), SpawnParams);
	if (!IsValid(Boss))
	{
		return;
	}

	Boss->WaveManagerRef = this;
	CurrentBossRef = Boss;
	BossSpawnedThisWave = true;
	AliveEnemyCount += 1;

	UpdateBossUI(true, 1.0f);
}

void AShootingWaveManager::NotifyEnemyRemoved(int32 ScoreAmount)
{
	CurrentScore += ScoreAmount;
	AliveEnemyCount = FMath::Max(AliveEnemyCount - 1, 0);
	RefreshHUD();

	if (AliveEnemyCount <= 0 && SpawnedCount >= EnemiesToSpawnThisWave && WaveRunning)
	{
		WaveRunning = false;
		StartMessage = FText::Format(FText::FromString(TEXT("WAVE {0} CLEAR")), FText::AsNumber(CurrentWave));
		RefreshHUD();

		GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AShootingWaveManager::ScheduleNextWave, 2.0f, false);
	}
}

void AShootingWaveManager::DamagePlayer(float DamageAmount)
{
	if (GameOver || PlayerInvincible)
	{
		return;
	}

	PlayerHP = FMath::Max(PlayerHP - DamageAmount, 0.0f);
	RefreshHUD();

	if (PlayerHP <= 0.0f)
	{
		GameOverEvent();
	}
}

void AShootingWaveManager::GameOverEvent()
{
	GameOver = true;
	WaveRunning = false;
	StartMessage = FText::GetEmpty();
	ShowBossUI = false;
	RefreshHUD();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		PlayerPawn->DisableInput(PlayerController);
	}

	UGameplayStatics::SetGamePaused(this, true);

	ShowGameOverUI = true;
	RefreshHUD();
}

void AShootingWaveManager::RefreshHUD()
{
	if (!IsValid(HUDRef))
	{
		return;
	}

	HUDRef->UpdateHUD(
		CurrentWave,
		CurrentScore,
		PlayerHP,
		StartMessage,
		ShowBossUI,
		BossHPPercent,
		ShowGameOverUI);

	HUDRef->UpdatePowerUpStatus(
		InvincibleRemainingTime,
		DamageMultiplier,
		DamageBuffRemainingTime);
}

void AShootingWaveManager::UpdateBossUI(bool bShowUI, float NewPercent)
{
	ShowBossUI = bShowUI;
	BossHPPercent = NewPercent;
	RefreshHUD();
}

void AShootingWaveManager::UpdatePowerUpTimers(float DeltaTime)
{
	if (InvincibleRemainingTime > 0.0f)
	{
		InvincibleRemainingTime = FMath::Max(InvincibleRemainingTime - DeltaTime, 0.0f);
		if (InvincibleRemainingTime <= 0.0f)
		{
			PlayerInvincible = false;
		}
	}

	if (DamageBuffRemainingTime > 0.0f)
	{
		DamageBuffRemainingTime = FMath::Max(DamageBuffRemainingTime - DeltaTime, 0.0f);
		if (DamageBuffRemainingTime <= 0.0f)
		{
			DamageMultiplier = 1.0f;
		}
	}

	RefreshHUD();
}

void AShootingWaveManager::ApplyHealPowerUp(float HealAmount)
{
	PlayerHP = FMath::Min(PlayerHP + HealAmount, MaxPlayerHP);
}

void AShootingWaveManager::ApplyInvinciblePowerUp(float Duration)
{
	PlayerInvincible = true;
	InvincibleRemainingTime = FMath::Max(InvincibleRemainingTime, Duration);
	RefreshHUD();
}

void AShootingWaveManager::ApplyDamagePowerUp(float Multiplier, float Duration)
{
	DamageMultiplier = Multiplier;
	DamageBuffRemainingTime = FMath::Max(DamageBuffRemainingTime, Duration);
	RefreshHUD();
}

void AShootingWaveManager::RestartGame()
{
	if (!GameOver)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, RestartLevelName);
}

void AShootingWaveManager::ClearStartMessageAndRefresh()
{
	StartMessage = FText::GetEmpty();
	RefreshHUD();
}

void AShootingWaveManager::ScheduleNextWave()
{
	StartWave();
}
