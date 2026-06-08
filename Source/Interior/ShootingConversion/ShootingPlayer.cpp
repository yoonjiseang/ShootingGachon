#include "ShootingPlayer.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ShootingBullet.h"
#include "ShootingWaveManager.h"

AShootingPlayer::AShootingPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	FirePosition = CreateDefaultSubobject<USceneComponent>(TEXT("FirePosition"));
	SetRootComponent(FirePosition);
}

void AShootingPlayer::BeginPlay()
{
	Super::BeginPlay();

	WaveManagerRef = Cast<AShootingWaveManager>(
		UGameplayStatics::GetActorOfClass(this, AShootingWaveManager::StaticClass()));
}

void AShootingPlayer::FireBullet()
{
	if (!BulletClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = FirePosition ? FirePosition->GetComponentTransform() : GetActorTransform();
	AShootingBullet* Bullet = GetWorld()->SpawnActor<AShootingBullet>(BulletClass, SpawnTransform, SpawnParams);
	if (!IsValid(Bullet))
	{
		return;
	}

	Bullet->Direction = SpawnTransform.GetRotation().GetForwardVector();
	Bullet->DamageAmount = BaseBulletDamage * (IsValid(WaveManagerRef) ? WaveManagerRef->DamageMultiplier : 1.0f);
}
