#include "ShootingBossEnemy.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "ShootingBossBullet.h"
#include "ShootingWaveManager.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

namespace
{
	bool TryReadDamageAmount(AActor* Actor, float& OutDamageAmount)
	{
		if (!Actor)
		{
			return false;
		}

		if (const FProperty* Property = Actor->GetClass()->FindPropertyByName(TEXT("DamageAmount")))
		{
			if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
			{
				OutDamageAmount = FloatProperty->GetPropertyValue_InContainer(Actor);
				return true;
			}

			if (const FIntProperty* IntProperty = CastField<FIntProperty>(Property))
			{
				OutDamageAmount = static_cast<float>(IntProperty->GetPropertyValue_InContainer(Actor));
				return true;
			}
		}

		return false;
	}
}

AShootingBossEnemy::AShootingBossEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	BossMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	BossMesh->SetupAttachment(CollisionComponent);
	BossMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BossMesh->SetGenerateOverlapEvents(false);
	BossMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	BossMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
	BossMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	BeamStart = CreateDefaultSubobject<USceneComponent>(TEXT("BeamStart"));
	BeamStart->SetupAttachment(CollisionComponent);
	BeamStart->SetRelativeLocation(FVector(0.0f, 0.0f, -31.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BossMeshAsset(TEXT("/Game/Drone/Drone_low.Drone_low"));
	if (BossMeshAsset.Succeeded())
	{
		BossMesh->SetStaticMesh(BossMeshAsset.Object);
	}

	ScoreValue = 1000;
	DistanceTolerance = 150.0f;
	MoveSpeed = 300.0f;
	BossBulletClass = AShootingBossBullet::StaticClass();

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DeathEffectAsset(
		TEXT("/Game/Fire_EXP_Vol01_Free/Niagara/EXP/NS_Sub_EXP_Large_001_01.NS_Sub_EXP_Large_001_01"));
	if (DeathEffectAsset.Succeeded())
	{
		DeathEffect = DeathEffectAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSoundAsset(TEXT("/Game/Explosion.Explosion"));
	if (DeathSoundAsset.Succeeded())
	{
		DeathSound = DeathSoundAsset.Object;
	}
}

void AShootingBossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingBossEnemy::OnCollisionBeginOverlap);
	}

	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);
	HP = MaxHP;
	BeamTimer = BeamCooldown;

	if (!BossBulletClass)
	{
		BossBulletClass = AShootingBossBullet::StaticClass();
	}

	if (IsValid(WaveManagerRef))
	{
		WaveManagerRef->UpdateBossUI(true, 1.0f);
	}
}

void AShootingBossEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsValid(PlayerRef))
	{
		PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);
	}

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
	if (!IsValid(PlayerRef))
	{
		PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	if (!IsValid(PlayerRef))
	{
		return;
	}

	TSubclassOf<AShootingBossBullet> BulletClassToSpawn = BossBulletClass;
	if (!BulletClassToSpawn)
	{
		BulletClassToSpawn = AShootingBossBullet::StaticClass();
	}

	FVector ToPlayer = PlayerRef->GetActorLocation() - GetActorLocation();
	ToPlayer.X = 0.0f;
	FVector DirectionToPlayer = ToPlayer.GetSafeNormal();
	if (DirectionToPlayer.IsNearlyZero())
	{
		DirectionToPlayer = FVector(0.0f, 0.0f, -1.0f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnBaseLocation = BeamStart ? BeamStart->GetComponentLocation() : GetActorLocation();
	const FVector SpawnLocation = SpawnBaseLocation + DirectionToPlayer * 90.0f;
	const FRotator SpawnRotation = FRotationMatrix::MakeFromX(DirectionToPlayer).Rotator();

	AShootingBossBullet* Bullet = GetWorld()->SpawnActor<AShootingBossBullet>(
		BulletClassToSpawn,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	if (!IsValid(Bullet))
	{
		return;
	}

	Bullet->Direction = DirectionToPlayer;
	Bullet->WaveManagerRef = WaveManagerRef;
	Bullet->DamageAmount = BeamDamage;
	Bullet->SetActorRotation(SpawnRotation);
}

void AShootingBossEnemy::HandleOverlap(AActor* OtherActor)
{
	float DamageAmount = 0.0f;
	if (TryReadDamageAmount(OtherActor, DamageAmount))
	{
		ApplyDamageToEnemy(FMath::TruncToFloat(DamageAmount));
		OtherActor->Destroy();
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
	FVector ToPlayer = PlayerRef->GetActorLocation() - GetActorLocation();
	ToPlayer.X = 0.0f;
	const float Distance = ToPlayer.Length();
	FVector DirectionToPlayer = ToPlayer.GetSafeNormal();
	if (DirectionToPlayer.IsNearlyZero())
	{
		DirectionToPlayer = FVector(0.0f, 0.0f, -1.0f);
	}

	FVector MoveDirection = FVector::ZeroVector;
	if (Distance < DesiredDistance - DistanceTolerance)
	{
		MoveDirection -= DirectionToPlayer;
	}
	else if (Distance > DesiredDistance + DistanceTolerance)
	{
		MoveDirection += DirectionToPlayer;
	}

	const FVector StrafeDirection(0.0f, -DirectionToPlayer.Z, DirectionToPlayer.Y);
	MoveDirection += StrafeDirection * FMath::Sin(GetWorld()->GetTimeSeconds() * 1.4f) * 0.75f;

	if (!MoveDirection.IsNearlyZero())
	{
		FVector NewLocation = GetActorLocation() + MoveDirection.GetSafeNormal() * MoveSpeed * DeltaSeconds;
		NewLocation.X = GetActorLocation().X;
		NewLocation.Y = FMath::Clamp(NewLocation.Y, -520.0f, 520.0f);
		NewLocation.Z = FMath::Clamp(NewLocation.Z, 200.0f, 1250.0f);
		SetActorLocation(NewLocation, false);
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
