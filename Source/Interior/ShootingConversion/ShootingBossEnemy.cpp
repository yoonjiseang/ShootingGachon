#include "ShootingBossEnemy.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "ShootingBossBullet.h"
#include "ShootingBullet.h"
#include "ShootingPlayer.h"
#include "ShootingWaveManager.h"

AShootingBossEnemy::AShootingBossEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	BeamStart = CreateDefaultSubobject<USceneComponent>(TEXT("BeamStart"));
	BeamStart->SetupAttachment(CollisionComponent);
}

void AShootingBossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingBossEnemy::OnCollisionBeginOverlap);
	}

	PlayerRef = Cast<AShootingPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	HP = MaxHP;
}

void AShootingBossEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Dead || !IsValid(PlayerRef))
	{
		return;
	}

	UpdateDistanceMovement(DeltaSeconds);
	UpdateFireTimer(DeltaSeconds);
}

void AShootingBossEnemy::ApplyDamageToEnemy(float DamageAmount)
{
	if (Dead)
	{
		return;
	}

	HP = FMath::Max(HP - DamageAmount, 0.0f);

	if (HP <= 0.0f)
	{
		Die(ScoreValue);
		return;
	}

	if (IsValid(WaveManagerRef))
	{
		const float Percent = MaxHP > 0.0f ? HP / MaxHP : 0.0f;
		WaveManagerRef->UpdateBossUI(true, Percent);
	}
}

void AShootingBossEnemy::Die(int32 ScoreToGive)
{
	if (Dead)
	{
		return;
	}

	Dead = true;
	PlayDeathFeedback();

	if (IsValid(WaveManagerRef))
	{
		WaveManagerRef->UpdateBossUI(false, 0.0f);
		WaveManagerRef->NotifyEnemyRemoved(ScoreToGive);
	}

	Destroy();
}

void AShootingBossEnemy::FireBossBullet()
{
	if (!BossBulletClass || !IsValid(PlayerRef))
	{
		return;
	}

	const FVector DirectionToPlayer = (PlayerRef->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FTransform SpawnTransform = BeamStart ? BeamStart->GetComponentTransform() : GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShootingBossBullet* Bullet = GetWorld()->SpawnActor<AShootingBossBullet>(BossBulletClass, SpawnTransform, SpawnParams);
	if (!IsValid(Bullet))
	{
		return;
	}

	Bullet->Direction = DirectionToPlayer;
	Bullet->WaveManagerRef = WaveManagerRef;
	Bullet->DamageAmount = BeamDamage;
	Bullet->SetActorRotation(DirectionToPlayer.Rotation());
}

void AShootingBossEnemy::HandleOverlap(AActor* OtherActor)
{
	if (AShootingBullet* Bullet = Cast<AShootingBullet>(OtherActor))
	{
		ApplyDamageToEnemy(FMath::TruncToFloat(Bullet->DamageAmount));
		Bullet->Destroy();
	}
}

void AShootingBossEnemy::OnCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleOverlap(OtherActor);
}

void AShootingBossEnemy::UpdateDistanceMovement(float DeltaSeconds)
{
	const FVector ToPlayer = PlayerRef->GetActorLocation() - GetActorLocation();
	const float Distance = ToPlayer.Length();
	const FVector DirectionToPlayer = ToPlayer.GetSafeNormal();

	if (Distance < DesiredDistance - DistanceTolerance)
	{
		SetActorLocation(GetActorLocation() - DirectionToPlayer * MoveSpeed * DeltaSeconds, false);
	}
	else if (Distance > DesiredDistance + DistanceTolerance)
	{
		SetActorLocation(GetActorLocation() + DirectionToPlayer * MoveSpeed * DeltaSeconds, false);
	}
}

void AShootingBossEnemy::UpdateFireTimer(float DeltaSeconds)
{
	BeamTimer += DeltaSeconds;

	if (BeamTimer >= BeamCooldown)
	{
		BeamTimer = 0.0f;
		FireBossBullet();
	}
}

void AShootingBossEnemy::PlayDeathFeedback()
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
