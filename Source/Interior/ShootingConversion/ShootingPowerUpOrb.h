#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingPowerUpType.h"
#include "ShootingPowerUpOrb.generated.h"

class AShootingWaveManager;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class INTERIOR_API AShootingPowerUpOrb : public AActor
{
	GENERATED_BODY()

public:
	AShootingPowerUpOrb();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power Up")
	TObjectPtr<UStaticMeshComponent> OrbMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power Up")
	TObjectPtr<UTextRenderComponent> EffectText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	EShootingPowerUpType EffectType = EShootingPowerUpType::DamageUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float DamageMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float BuffDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float HealAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> SourceMaterial;

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInstanceDynamic> OrbMaterial;

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void ApplyVisualByType();

	UFUNCTION(BlueprintCallable, Category = "Power Up")
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
