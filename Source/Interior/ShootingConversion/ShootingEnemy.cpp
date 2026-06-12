#include "ShootingEnemy.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "ShootingPlayer.h"
#include "ShootingPowerUpOrb.h"
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

AShootingEnemy::AShootingEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	EnemyMesh->SetupAttachment(CollisionComponent);
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyMesh->SetGenerateOverlapEvents(false);
	EnemyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	EnemyMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));
	EnemyMesh->SetRelativeScale3D(FVector(0.332881f, 0.332881f, 0.332881f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> EnemyMeshAsset(TEXT("/Game/Drone/Drone_low.Drone_low"));
	if (EnemyMeshAsset.Succeeded())
	{
		EnemyMesh->SetStaticMesh(EnemyMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMaterialAsset(TEXT("/Game/Drone/M_Drone.M_Drone"));
	if (EnemyMaterialAsset.Succeeded())
	{
		EnemyMesh->SetMaterial(0, EnemyMaterialAsset.Object);
	}

	ScoreValue = 100;
	PowerUpOrbClass = AShootingPowerUpOrb::StaticClass();

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

void AShootingEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingEnemy::OnCollisionBeginOverlap);
	}

	PlayerRef = Cast<AShootingPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	SpawnDepthX = GetActorLocation().X;
	SpawnBaseY = GetActorLocation().Y;
	DriftPhase = FMath::FRandRange(0.0f, 6.283185f);
}

void AShootingEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Dead)
	{
		return;
	}

	FVector NewLocation = GetActorLocation();
	NewLocation.X = SpawnDepthX;

	FVector MoveDirection(0.0f, 0.0f, -1.0f);
	if (IsValid(PlayerRef))
	{
		FVector ToPlayer = PlayerRef->GetActorLocation() - GetActorLocation();
		ToPlayer.X = 0.0f;
		const FVector DirectionToPlayer = ToPlayer.GetSafeNormal();
		if (!DirectionToPlayer.IsNearlyZero())
		{
			MoveDirection = FMath::Lerp(FVector(0.0f, 0.0f, -1.0f), DirectionToPlayer, PlayerChaseWeight).GetSafeNormal();
		}
	}

	NewLocation += MoveDirection * MoveSpeed * DeltaSeconds;

	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewLocation.Y += FMath::Sin(Time * HorizontalDriftSpeed + DriftPhase) * HorizontalDriftAmplitude * DeltaSeconds;

	NewLocation.X = SpawnDepthX;
	NewLocation.Y = FMath::Clamp(NewLocation.Y, -520.0f, 520.0f);
	SetActorLocation(NewLocation, false);

	if (GetActorLocation().Z < EscapeZ)
	{
		RemoveEscapedEnemy();
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
	float DamageAmount = 0.0f;
	if (TryReadDamageAmount(OtherActor, DamageAmount))
	{
		ApplyDamageToEnemy(FMath::TruncToFloat(DamageAmount));
		OtherActor->Destroy();
		return;
	}

	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
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

void AShootingEnemy::RemoveEscapedEnemy()
{
	if (Dead)
	{
		return;
	}

	Dead = true;

	if (IsValid(WaveManagerRef))
	{
		WaveManagerRef->NotifyEnemyRemoved(0);
	}

	Destroy();
}
