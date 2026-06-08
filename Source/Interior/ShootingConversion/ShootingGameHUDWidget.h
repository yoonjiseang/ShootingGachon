#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShootingGameHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;

UCLASS(Blueprintable)
class INTERIOR_API UShootingGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUD(
		int32 InWave,
		int32 InScore,
		float InHP,
		const FText& InMessage,
		bool bInShowBoss,
		float InBossPercent,
		bool bInShowGameOver);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdatePowerUpStatus(
		float InInvincibleTime,
		float InDamageMultiplier,
		float InDamageTime);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTWave;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTScore;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTHP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTMessage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTBoss;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UProgressBar> PBBossHP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UImage> IMGGameOverFade;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTGameOverTitle;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTRestartHint;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTInvincibleStatus;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "HUD")
	TObjectPtr<UTextBlock> TXTAttackStatus;
};
