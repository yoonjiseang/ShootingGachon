#include "ShootingBossBullet.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ShootingWaveManager.h"
#include "UObject/ConstructorHelpers.h"

AShootingBossBullet::AShootingBossBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	CollisionComponent->SetRelativeScale3D(FVector(1.0f, 0.25f, 0.25f));
	InitialLifeSpan = 6.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BulletMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (BulletMeshAsset.Succeeded())
	{
		CollisionComponent->SetStaticMesh(BulletMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BulletMaterialAsset(
		TEXT("/Game/Game/Material/M_PowerUpOrb.M_PowerUpOrb"));
	if (BulletMaterialAsset.Succeeded())
	{
		CollisionComponent->SetMaterial(0, BulletMaterialAsset.Object);
	}
}

void AShootingBossBullet::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* BulletMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Game/Material/M_PowerUpOrb.M_PowerUpOrb"));
	if (BulletMaterial)
	{
		CollisionComponent->SetMaterial(0, BulletMaterial);
	}

	if (UMaterialInstanceDynamic* DynamicMaterial = CollisionComponent->CreateDynamicMaterialInstance(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("OrbColor"), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
	}

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShootingBossBullet::OnCollisionBeginOverlap);
	}
}

void AShootingBossBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector NewLocation = GetActorLocation() + Direction.GetSafeNormal() * Speed * DeltaSeconds;
	SetActorLocation(NewLocation, true);
}

void AShootingBossBullet::HandlePlayerOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
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
