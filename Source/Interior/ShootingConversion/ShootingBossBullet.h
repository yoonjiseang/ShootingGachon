#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingBossBullet.generated.h"

class AShootingPlayer;
class AShootingWaveManager;
class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingBossBullet : public AActor
{
	GENERATED_BODY()

public:
	AShootingBossBullet();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Bullet")
	TObjectPtr<UStaticMeshComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Bullet")
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Bullet")
	float Speed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Bullet")
	float DamageAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Bullet")
	TObjectPtr<AShootingWaveManager> WaveManagerRef;

	UFUNCTION(BlueprintCallable, Category = "Boss Bullet")
	void HandlePlayerOverlap(AActor* OtherActor);

protected:
	UFUNCTION()
	void OnCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
