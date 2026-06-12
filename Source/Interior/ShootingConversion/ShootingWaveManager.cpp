#include "ShootingWaveManager.h"

#include "Components/Image.h"
#include "Components/InputComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ShootingBossEnemy.h"
#include "ShootingEnemy.h"
#include "TimerManager.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Class.h"

namespace
{
	template <typename TWidgetType>
	TWidgetType* FindNamedWidget(UUserWidget* UserWidget, std::initializer_list<const TCHAR*> CandidateNames)
	{
		if (!IsValid(UserWidget))
		{
			return nullptr;
		}

		for (const TCHAR* CandidateName : CandidateNames)
		{
			if (TWidgetType* FoundWidget = Cast<TWidgetType>(UserWidget->GetWidgetFromName(FName(CandidateName))))
			{
				return FoundWidget;
			}
		}

		return nullptr;
	}

	template <typename TWidgetType>
	TWidgetType* FindWidgetByNameParts(UUserWidget* UserWidget, std::initializer_list<const TCHAR*> RequiredNameParts)
	{
		if (!IsValid(UserWidget) || !UserWidget->WidgetTree)
		{
			return nullptr;
		}

		TArray<UWidget*> Widgets;
		UserWidget->WidgetTree->GetAllWidgets(Widgets);
		for (UWidget* Widget : Widgets)
		{
			TWidgetType* TypedWidget = Cast<TWidgetType>(Widget);
			if (!TypedWidget)
			{
				continue;
			}

			const FString WidgetName = Widget->GetName();
			bool bMatchesAllParts = true;
			for (const TCHAR* RequiredNamePart : RequiredNameParts)
			{
				if (!WidgetName.Contains(RequiredNamePart, ESearchCase::IgnoreCase))
				{
					bMatchesAllParts = false;
					break;
				}
			}

			if (bMatchesAllParts)
			{
				return TypedWidget;
			}
		}

		return nullptr;
	}

	FText FormatSecondsTwoDecimals(float Seconds)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.SetMinimumFractionalDigits(2);
		FormatOptions.SetMaximumFractionalDigits(2);
		return FText::AsNumber(FMath::Max(Seconds, 0.0f), &FormatOptions);
	}

	FText FormatMultiplierOneDecimal(float Multiplier)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.SetMinimumFractionalDigits(1);
		FormatOptions.SetMaximumFractionalDigits(1);
		return FText::AsNumber(Multiplier, &FormatOptions);
	}
}

AShootingWaveManager::AShootingWaveManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShootingWaveManager::BeginPlay()
{
	Super::BeginPlay();

	ResetRuntimeStateForNewGame();

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
		HUDRef = CreateWidget<UUserWidget>(PlayerController, HUDWidgetClass);
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

void AShootingWaveManager::ResetRuntimeStateForNewGame()
{
	if (MaxPlayerHP <= 0.0f)
	{
		MaxPlayerHP = 3.0f;
	}

	if (SpawnCooldown <= 0.0f)
	{
		SpawnCooldown = 1.0f;
	}

	if (BossWaveInterval <= 0)
	{
		BossWaveInterval = 3;
	}

	CurrentBossRef = nullptr;
	CurrentWave = 0;
	CurrentScore = 0;
	EnemiesToSpawnThisWave = 0;
	SpawnedCount = 0;
	AliveEnemyCount = 0;
	SpawnTimer = 0.0f;
	WaveRunning = false;
	IsBossWave = false;
	BossSpawnedThisWave = false;
	ShowBossUI = false;
	BossHPPercent = 1.0f;
	PlayerHP = MaxPlayerHP;
	PlayerInvincible = false;
	InvincibleRemainingTime = 0.0f;
	DamageMultiplier = 1.0f;
	DamageBuffRemainingTime = 0.0f;
	StartMessage = FText::GetEmpty();
	ShowGameOverUI = false;
	GameOver = false;
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

	if (UTextBlock* WaveText = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_Wave"), TEXT("TXTWave")}))
	{
		WaveText->SetText(FText::Format(FText::FromString(TEXT("Wave: {0}")), FText::AsNumber(CurrentWave)));
	}

	if (UTextBlock* ScoreText = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_Score"), TEXT("TXTScore")}))
	{
		ScoreText->SetText(FText::Format(FText::FromString(TEXT("Score: {0}")), FText::AsNumber(CurrentScore)));
	}

	if (UTextBlock* HPText = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_HP"), TEXT("TXTHP")}))
	{
		HPText->SetText(FText::Format(FText::FromString(TEXT("HP: {0}")), FText::AsNumber(FMath::RoundToInt(PlayerHP))));
	}

	if (UTextBlock* MessageText = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_Message"), TEXT("TXTMessage")}))
	{
		MessageText->SetText(StartMessage);
	}

	const ESlateVisibility BossVisibility = ShowBossUI ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	UTextBlock* BossText = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_Boss"), TEXT("TXTBoss"), TEXT("TXT Boss")});
	if (!BossText)
	{
		BossText = FindWidgetByNameParts<UTextBlock>(HUDRef, {TEXT("Boss")});
	}
	if (BossText)
	{
		BossText->SetVisibility(BossVisibility);
	}

	UProgressBar* BossHPBar = FindNamedWidget<UProgressBar>(HUDRef, {TEXT("PB_Boss_HP"), TEXT("PBBossHP"), TEXT("PB Boss HP")});
	if (!BossHPBar)
	{
		BossHPBar = FindWidgetByNameParts<UProgressBar>(HUDRef, {TEXT("Boss")});
	}
	if (!BossHPBar)
	{
		BossHPBar = FindWidgetByNameParts<UProgressBar>(HUDRef, {TEXT("HP")});
	}
	if (BossHPBar)
	{
		BossHPBar->SetPercent(FMath::Clamp(BossHPPercent, 0.0f, 1.0f));
		BossHPBar->SetVisibility(BossVisibility);
		BossHPBar->SetRenderOpacity(1.0f);
	}

	const ESlateVisibility GameOverVisibility = ShowGameOverUI ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (UImage* GameOverFade = FindNamedWidget<UImage>(HUDRef, {TEXT("IMG_GameOverFade"), TEXT("IMGGameOverFade")}))
	{
		GameOverFade->SetVisibility(GameOverVisibility);
	}

	if (UTextBlock* GameOverTitle = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_GameOverTitle"), TEXT("TXTGameOverTitle")}))
	{
		GameOverTitle->SetVisibility(GameOverVisibility);
	}

	if (UTextBlock* RestartHint = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_RestartHint"), TEXT("TXTRestartHint")}))
	{
		RestartHint->SetVisibility(GameOverVisibility);
	}

	if (UTextBlock* InvincibleStatus = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_InvincibleStatus"), TEXT("TXTInvincibleStatus"), TEXT("TXT Invincible Status")}))
	{
		if (InvincibleRemainingTime > 0.0f)
		{
			InvincibleStatus->SetVisibility(ESlateVisibility::Visible);
			InvincibleStatus->SetText(FText::Format(
				FText::FromString(TEXT("\uBB34\uC801 {0}s")),
				FText::AsNumber(FMath::CeilToInt(InvincibleRemainingTime))));
		}
		else
		{
			InvincibleStatus->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (UTextBlock* AttackStatus = FindNamedWidget<UTextBlock>(HUDRef, {TEXT("TXT_AttackStatus"), TEXT("TXTAttackStatus"), TEXT("TXT Attack Status")}))
	{
		AttackStatus->SetVisibility(ESlateVisibility::Visible);
		if (DamageBuffRemainingTime > 0.0f)
		{
			AttackStatus->SetText(FText::Format(
				FText::FromString(TEXT("\uACF5\uACA9 x{0} ({1}s)")),
				FormatMultiplierOneDecimal(DamageMultiplier),
				FormatSecondsTwoDecimals(DamageBuffRemainingTime)));
		}
		else
		{
			AttackStatus->SetText(FText::FromString(TEXT("\uACF5\uACA9 x1.0")));
		}
	}
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
	RefreshHUD();
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
