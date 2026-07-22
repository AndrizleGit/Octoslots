#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "Projectiles/BaseProjectile.h"
#include "CrabBomb.generated.h"

UCLASS()
class OCTOPIRATE_API ACrabBomb : public ABaseProjectile
{
	GENERATED_BODY()

public:
	ACrabBomb();
	
	// -- Sound --
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> ExplosionSound;
	
	UFUNCTION(BlueprintCallable, Category = "Projectile|Bomb")
	void LaunchAtTarget(const FVector& TargetLocation, float ArcParam = 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Bomb")
	float ExplosionRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Bomb")
	TObjectPtr<class UNiagaraSystem> ExplosionVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Bomb")
	float DeflectThrowDistance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Bomb")
	float DeflectArcParam = 0.5f;

	virtual void OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleImpact(AActor* OtherActor, const FHitResult& Hit) override;
	virtual FVector ComputeDeflectedVelocity(const FVector& ReflectedVelocity, AActor* Deflector) const override;

	UFUNCTION()
	void OnPawnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void Explode();
};