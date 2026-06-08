#include "ShootingPowerUpOrb.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ShootingPlayer.h"
#include "ShootingWaveManager.h"

AShootingPowerUpOrb::AShootingPowerUpOrb()
{
	PrimaryActorTick.bCanEverTick = false;

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	SetRootComponent(OrbMesh);
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OrbMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	OrbMesh->SetGenerateOverlapEvents(true);

	EffectText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("EffectText"));
	EffectText->SetupAttachment(OrbMesh);
	EffectText->SetHorizontalAlignment(EHTA_Center);
	EffectText->SetTextRenderColor(FColor::White);
}

void AShootingPowerUpOrb::BeginPlay()
{
	Super::BeginPlay();

	if (OrbMesh)
	{
		OrbMesh->OnComponentBeginOverlap.AddDynamic(this, &AShootingPowerUpOrb::OnCollisionBeginOverlap);
		OrbMaterial = OrbMesh->CreateDynamicMaterialInstance(0, SourceMaterial);
	}

	ApplyVisualByType();
}

void AShootingPowerUpOrb::ApplyVisualByType()
{
	FText Label = FText::FromString(TEXT("DMG"));
	FLinearColor Color(1.0f, 0.1f, 0.05f, 1.0f);

	switch (EffectType)
	{
	case EShootingPowerUpType::DamageUp:
		Label = FText::FromString(TEXT("DMG"));
		Color = FLinearColor(1.0f, 0.1f, 0.05f, 1.0f);
		break;
	case EShootingPowerUpType::Invincible:
		Label = FText::FromString(TEXT("INV"));
		Color = FLinearColor(0.1f, 0.5f, 1.0f, 1.0f);
		break;
	case EShootingPowerUpType::Heal:
		Label = FText::FromString(TEXT("HEAL"));
		Color = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);
		break;
	default:
		break;
	}

	if (EffectText)
	{
		EffectText->SetText(Label);
	}

	if (OrbMaterial)
	{
		OrbMaterial->SetVectorParameterValue(TEXT("OrbColor"), Color);
	}
}

void AShootingPowerUpOrb::HandlePlayerOverlap(AActor* OtherActor)
{
	if (!OtherActor || !OtherActor->IsA<AShootingPlayer>())
	{
		return;
	}

	AShootingWaveManager* WaveManager = Cast<AShootingWaveManager>(
		UGameplayStatics::GetActorOfClass(this, AShootingWaveManager::StaticClass()));

	if (!IsValid(WaveManager))
	{
		return;
	}

	switch (EffectType)
	{
	case EShootingPowerUpType::DamageUp:
		WaveManager->ApplyDamagePowerUp(DamageMultiplier, BuffDuration);
		break;
	case EShootingPowerUpType::Invincible:
		WaveManager->ApplyInvinciblePowerUp(BuffDuration);
		break;
	case EShootingPowerUpType::Heal:
		WaveManager->ApplyHealPowerUp(HealAmount);
		break;
	default:
		break;
	}

	Destroy();
	WaveManager->RefreshHUD();
}

void AShootingPowerUpOrb::OnCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandlePlayerOverlap(OtherActor);
}
