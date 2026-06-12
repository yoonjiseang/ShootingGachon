#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootingBullet.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingBullet : public APawn
{
	GENERATED_BODY()

public:
	AShootingBullet();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TObjectPtr<UStaticMeshComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	FVector Direction = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float Speed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float DamageAmount = 1.0f;
};
