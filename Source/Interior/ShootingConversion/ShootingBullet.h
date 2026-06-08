#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingBullet.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingBullet : public AActor
{
	GENERATED_BODY()

public:
	AShootingBullet();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TObjectPtr<UStaticMeshComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float Speed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float DamageAmount = 1.0f;
};
