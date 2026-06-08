#include "ShootingGameHUDWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UShootingGameHUDWidget::UpdateHUD(
	int32 InWave,
	int32 InScore,
	float InHP,
	const FText& InMessage,
	bool bInShowBoss,
	float InBossPercent,
	bool bInShowGameOver)
{
	if (TXTWave)
	{
		TXTWave->SetText(FText::Format(FText::FromString(TEXT("Wave: {0}")), FText::AsNumber(InWave)));
	}

	if (TXTScore)
	{
		TXTScore->SetText(FText::Format(FText::FromString(TEXT("Score: {0}")), FText::AsNumber(InScore)));
	}

	if (TXTHP)
	{
		TXTHP->SetText(FText::Format(FText::FromString(TEXT("HP: {0}")), FText::AsNumber(InHP)));
	}

	if (TXTMessage)
	{
		TXTMessage->SetText(InMessage);
	}

	if (PBBossHP)
	{
		PBBossHP->SetPercent(FMath::Clamp(InBossPercent, 0.0f, 1.0f));
		PBBossHP->SetVisibility(bInShowBoss ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (TXTBoss)
	{
		TXTBoss->SetVisibility(bInShowBoss ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	const ESlateVisibility GameOverVisibility = bInShowGameOver ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (IMGGameOverFade)
	{
		IMGGameOverFade->SetVisibility(GameOverVisibility);
	}
	if (TXTGameOverTitle)
	{
		TXTGameOverTitle->SetVisibility(GameOverVisibility);
	}
	if (TXTRestartHint)
	{
		TXTRestartHint->SetVisibility(GameOverVisibility);
	}
}

void UShootingGameHUDWidget::UpdatePowerUpStatus(
	float InInvincibleTime,
	float InDamageMultiplier,
	float InDamageTime)
{
	if (TXTInvincibleStatus)
	{
		if (InInvincibleTime > 0.0f)
		{
			TXTInvincibleStatus->SetVisibility(ESlateVisibility::Visible);
			TXTInvincibleStatus->SetText(FText::Format(
				FText::FromString(TEXT("무적 {0}s")),
				FText::AsNumber(FMath::CeilToInt(InInvincibleTime))));
		}
		else
		{
			TXTInvincibleStatus->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (TXTAttackStatus)
	{
		TXTAttackStatus->SetVisibility(ESlateVisibility::Visible);
		if (InDamageTime > 0.0f)
		{
			TXTAttackStatus->SetText(FText::Format(
				FText::FromString(TEXT("공격 x{0} ({1}s)")),
				FText::AsNumber(InDamageMultiplier),
				FText::AsNumber(FMath::CeilToInt(InDamageTime))));
		}
		else
		{
			TXTAttackStatus->SetText(FText::FromString(TEXT("공격 x1.0")));
		}
	}
}
