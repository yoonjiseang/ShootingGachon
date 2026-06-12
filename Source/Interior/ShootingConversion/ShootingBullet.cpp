#include "ShootingBullet.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AShootingBullet::AShootingBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetRelativeScale3D(FVector(1.0f, 0.25f, 0.5f));
	InitialLifeSpan = 4.0f;

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

void AShootingBullet::BeginPlay()
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
		DynamicMaterial->SetVectorParameterValue(TEXT("OrbColor"), FLinearColor(0.0f, 1.0f, 0.15f, 1.0f));
	}
}

void AShootingBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector NewLocation = GetActorLocation() + Direction.GetSafeNormal() * Speed * DeltaSeconds;
	SetActorLocation(NewLocation, true);
}
