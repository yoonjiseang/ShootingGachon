#include "ShootingBossBullet.h"

#include "Components/StaticMeshComponent.h"
#include "ShootingPlayer.h"
#include "ShootingWaveManager.h"

AShootingBossBullet::AShootingBossBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AShootingBossBullet::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingBossBullet::OnCollisionBeginOverlap);
	}
}

void AShootingBossBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector NewLocation = GetActorLocation() + Direction.GetSafeNormal() * Speed * DeltaSeconds;
	SetActorLocation(NewLocation, false);
}

void AShootingBossBullet::HandlePlayerOverlap(AActor* OtherActor)
{
	if (!OtherActor || !OtherActor->IsA<AShootingPlayer>())
	{
		return;
	}

	if (IsValid(WaveManagerRef))
	{
		WaveManagerRef->DamagePlayer(DamageAmount);
	}

	Destroy();
}

void AShootingBossBullet::OnCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandlePlayerOverlap(OtherActor);
}
