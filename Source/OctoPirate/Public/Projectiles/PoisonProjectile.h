#pragma once

#include "CoreMinimal.h"
#include "Projectiles/BaseProjectile.h"
#include "PoisonProjectile.generated.h"

class UNiagaraComponent;

/** Which poison a ball delivers to the enemies it passes through. */
UENUM(BlueprintType)
enum class EPoisonProjectileEffect : uint8
{
	// The normal Buffs.PoisonWeapon effect, applied once per buff stack.
	Standard        UMETA(DisplayName = "Standard Poison"),
	// The stronger three-of-a-kind poison.
	ThreeOfAKind    UMETA(DisplayName = "Three Of A Kind Poison")
};

/**
 * Poison ball fired forward on every attack swing while the Buffs.PoisonWeapon
 * buff is active. It flies in a straight line for TravelDistance, slowing down
 * as it goes, and applies the player's poison GameplayEffect to every enemy it
 * passes through (once per enemy). It does not stop on impact.
 */
UCLASS()
class OCTOPIRATE_API APoisonProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	APoisonProjectile();

	// Which poison this ball applies. The player overrides this when it fires a
	// volley, so it only matters for balls spawned by hand.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile")
	EPoisonProjectileEffect PoisonEffectMode = EPoisonProjectileEffect::Standard;

	// How far the ball travels before it stops and cleans itself up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile|Travel")
	float TravelDistance = 800.f;

	// Speed at the moment it is spawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile|Travel")
	float LaunchSpeed = 1200.f;

	// Speed at the end of the travel, as a fraction of LaunchSpeed.
	// 1 = no slowdown, 0.15 = ends at 15% of the launch speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile|Travel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EndSpeedFraction = 0.15f;

	// Shape of the slowdown over the travel distance.
	// 1 = linear, >1 = holds its speed and drops off near the end, <1 = slows down early.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile|Travel", meta = (ClampMin = "0.05"))
	float SpeedFalloffExponent = 2.f;

	// Time the VFX is left to finish after the ball has stopped, before the actor is destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Projectile|Travel", meta = (ClampMin = "0.0"))
	float VFXLingerTime = 0.5f;

	// The poison trail/ball VFX. Assign NS_PoisonBall on this component in the Blueprint.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poison Projectile")
	TObjectPtr<UNiagaraComponent> PoisonVFX;

	// Normalized 0..1 progress along the travel distance. Handy for BP-driven VFX params.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Poison Projectile")
	float GetTravelProgress() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnPoisonOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Applies the owning player's poison effect to Target, at most once per actor.
	void TryPoisonActor(AActor* Target);

	// Stops the ball, disables collision and lets the VFX fade out before destroying the actor.
	void CompleteTravel();

private:
	FVector TravelOrigin = FVector::ZeroVector;
	FVector TravelDirection = FVector::ForwardVector;
	bool bTravelComplete = false;

	// Enemies already poisoned by this ball, so passing through one only poisons it once.
	TSet<TWeakObjectPtr<AActor>> PoisonedActors;
};
