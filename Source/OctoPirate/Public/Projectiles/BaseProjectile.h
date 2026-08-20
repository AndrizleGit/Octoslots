#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class OCTOPIRATE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	// called by the deflect system when this projectile is deflected
	// override in subclasses to implement custom deflect behavior
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OnDeflected(const FVector& ReflectedVelocity, AActor* Deflector);
	virtual void OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector);

	// whether this projectile can be deflected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanBeDeflected = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bDealDamageOnHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileDamage = 10.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	// spins the visual mesh while the projectile is in the air.
	// only the mesh spins, so the travel direction is unaffected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Spin")
	bool bSpinMeshWhileTraveling = false;

	// spin applied to the mesh in its local space, in degrees per second.
	// Pitch tumbles it end over end, Roll barrel-rolls it, Yaw spins it flat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Spin", meta = (EditCondition = "bSpinMeshWhileTraveling"))
	FRotator SpinRate = FRotator(720.f, 0.f, 0.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Projectile")
	bool WasDeflected() const { return bIsDeflected; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	// whether this projectile has already been deflected
	bool bIsDeflected = false;
	
	virtual void HandleImpact(AActor* OtherActor, const FHitResult& Hit);
	
	// Computes the velocity to apply after a deflect
	virtual FVector ComputeDeflectedVelocity(const FVector& ReflectedVelocity, AActor* Deflector) const { return ReflectedVelocity; }
	
	bool bLockToSpawnHeight = true;
	float SpawnZ = 0.f;

	// the mesh the spin is applied to, resolved on BeginPlay
	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> SpinMesh;
private:
	
	FVector LastFrameVelocity = FVector::ZeroVector;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
}; 