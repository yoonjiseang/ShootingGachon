#include "ShootingPlayer.h"

#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ShootingBullet.h"
#include "ShootingWaveManager.h"
#include "UObject/ConstructorHelpers.h"

AShootingPlayer::AShootingPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	ShipMesh->SetupAttachment(CollisionComponent);
	ShipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShipMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	ShipMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
	ShipMesh->SetRelativeScale3D(FVector(0.086459f, 0.086459f, 0.086459f));

	FirePosition = CreateDefaultSubobject<USceneComponent>(TEXT("FirePosition"));
	FirePosition->SetupAttachment(CollisionComponent);
	FirePosition->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMeshAsset(TEXT("/Game/SpaceShip/Spaceship_ARA.Spaceship_ARA"));
	if (ShipMeshAsset.Succeeded())
	{
		ShipMesh->SetStaticMesh(ShipMeshAsset.Object);
	}

	BulletClass = AShootingBullet::StaticClass();
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AShootingPlayer::BeginPlay()
{
	Super::BeginPlay();

	PlayfieldDepthX = GetActorLocation().X;

	WaveManagerRef = Cast<AShootingWaveManager>(
		UGameplayStatics::GetActorOfClass(this, AShootingWaveManager::StaticClass()));

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->Possess(this);
		EnableInput(PlayerController);
		if (AActor* CameraActor = UGameplayStatics::GetActorOfClass(this, ACameraActor::StaticClass()))
		{
			PlayerController->SetViewTarget(CameraActor);
		}
	}
}

void AShootingPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FireTimer = FMath::Max(FireTimer - DeltaSeconds, 0.0f);

	float PolledHorizontalInput = HorizontalInput;
	float PolledVerticalInput = VerticalInput;

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PlayerController->GetPawn() != this)
		{
			PlayerController->Possess(this);
			if (AActor* CameraActor = UGameplayStatics::GetActorOfClass(this, ACameraActor::StaticClass()))
			{
				PlayerController->SetViewTarget(CameraActor);
			}
		}

		PolledHorizontalInput = 0.0f;
		PolledVerticalInput = 0.0f;

		if (PlayerController->IsInputKeyDown(EKeys::A) || PlayerController->IsInputKeyDown(EKeys::Left))
		{
			PolledHorizontalInput -= 1.0f;
		}
		if (PlayerController->IsInputKeyDown(EKeys::D) || PlayerController->IsInputKeyDown(EKeys::Right))
		{
			PolledHorizontalInput += 1.0f;
		}
		if (PlayerController->IsInputKeyDown(EKeys::W) || PlayerController->IsInputKeyDown(EKeys::Up))
		{
			PolledVerticalInput += 1.0f;
		}
		if (PlayerController->IsInputKeyDown(EKeys::S) || PlayerController->IsInputKeyDown(EKeys::Down))
		{
			PolledVerticalInput -= 1.0f;
		}

		if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton) || PlayerController->IsInputKeyDown(EKeys::SpaceBar))
		{
			FireBullet();
		}
	}

	const FVector MovementInput(0.0f, PolledHorizontalInput, PolledVerticalInput);
	if (!MovementInput.IsNearlyZero())
	{
		FVector NewLocation = GetActorLocation() + MovementInput.GetClampedToMaxSize(1.0f) * MoveSpeed * DeltaSeconds;
		NewLocation.X = PlayfieldDepthX;
		NewLocation.Y = FMath::Clamp(NewLocation.Y, MinY, MaxY);
		NewLocation.Z = FMath::Clamp(NewLocation.Z, MinZ, MaxZ);
		SetActorLocation(NewLocation, false);
	}
}

void AShootingPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindAxis(TEXT("Horizontal"), this, &AShootingPlayer::MoveHorizontal);
	PlayerInputComponent->BindAxis(TEXT("Vertical"), this, &AShootingPlayer::MoveVertical);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AShootingPlayer::FireBullet);
}

void AShootingPlayer::FireBullet()
{
	if (!BulletClass || FireTimer > 0.0f)
	{
		return;
	}

	FireTimer = FireCooldown;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = FirePosition ? FirePosition->GetComponentTransform() : GetActorTransform();
	AShootingBullet* Bullet = GetWorld()->SpawnActor<AShootingBullet>(BulletClass, SpawnTransform, SpawnParams);
	if (!IsValid(Bullet))
	{
		return;
	}

	Bullet->Direction = FVector::UpVector;
	Bullet->DamageAmount = BaseBulletDamage * (IsValid(WaveManagerRef) ? WaveManagerRef->DamageMultiplier : 1.0f);
}

void AShootingPlayer::MoveHorizontal(float Value)
{
	HorizontalInput = Value;
}

void AShootingPlayer::MoveVertical(float Value)
{
	VerticalInput = Value;
}
