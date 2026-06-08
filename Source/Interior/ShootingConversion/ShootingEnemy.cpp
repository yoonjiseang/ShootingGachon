#include "ShootingEnemy.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "ShootingBullet.h"
#include "ShootingPlayer.h"
#include "ShootingPowerUpOrb.h"
#include "ShootingWaveManager.h"

AShootingEnemy::AShootingEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AShootingEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingEnemy::OnCollisionBeginOverlap);
	}
}

void AShootingEnemy::ApplyDamageToEnemy(float DamageAmount)
{
	if (Dead)
	{
		return;
	}

	HP -= DamageAmount;

	if (HP <= 0.0f)
	{
		Die(ScoreValue);
	}
}

void AShootingEnemy::Die(int32 ScoreToGive)
{
	if (Dead)
	{
		return;
	}

	Dead = true;

	TryDropPowerUp();
	PlayDeathFeedback();

	if (IsValid(WaveManagerRef))
	{
		WaveManagerRef->NotifyEnemyRemoved(ScoreToGive);
	}

	Destroy();
}

void AShootingEnemy::TryDropPowerUp()
{
	if (!PowerUpOrbClass || FMath::FRandRange(0.0f, 1.0f) > PowerUpDropChance)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShootingPowerUpOrb* Orb = GetWorld()->SpawnActor<AShootingPowerUpOrb>(PowerUpOrbClass, GetActorTransform(), SpawnParams);
	if (!IsValid(Orb))
	{
		return;
	}

	const int32 RandomType = FMath::RandRange(0, 2);
	Orb->EffectType = static_cast<EShootingPowerUpType>(RandomType);
	Orb->ApplyVisualByType();
}

void AShootingEnemy::HandleOverlap(AActor* OtherActor)
{
	if (AShootingBullet* Bullet = Cast<AShootingBullet>(OtherActor))
	{
		ApplyDamageToEnemy(FMath::TruncToFloat(Bullet->DamageAmount));
		Bullet->Destroy();
		return;
	}

	if (OtherActor && OtherActor->IsA<AShootingPlayer>())
	{
		if (IsValid(WaveManagerRef))
		{
			WaveManagerRef->DamagePlayer(1.0f);
		}

		Die(0);
	}
}

void AShootingEnemy::OnCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleOverlap(OtherActor);
}

void AShootingEnemy::PlayDeathFeedback()
{
	if (DeathEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DeathEffect, GetActorLocation());
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySound2D(this, DeathSound);
	}
}
