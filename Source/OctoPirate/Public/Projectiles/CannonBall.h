#pragma once

#include "CoreMinimal.h"
#include "Projectiles/BaseProjectile.h"
#include "CannonBall.generated.h"

UCLASS()
class OCTOPIRATE_API ACannonBall : public ABaseProjectile
{
	GENERATED_BODY()

public:
	ACannonBall();

	// speed multiplier applied after deflection — 0.5 = half speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CannonBall")
	float PostDeflectSpeedMultiplier = 0.5f;

	virtual void OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector) override;
};