#include "Projectiles/CannonBall.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACannonBall::ACannonBall()
{
	bCanBeDeflected = true;
	bDealDamageOnHit = true;
	ProjectileDamage = 25.f;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bShouldBounce = false; 
}

void ACannonBall::OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector)
{
	if (!bCanBeDeflected || bIsDeflected) return;

	bIsDeflected = true;
	SetOwner(Deflector);
	SetInstigator(Cast<APawn>(Deflector));
	CollisionSphere->IgnoreActorWhenMoving(Deflector, true);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// update spawn Z to current height after deflection
	SpawnZ = GetActorLocation().Z;

	const FVector FlatVelocity = FVector(ReflectedVelocity.X, ReflectedVelocity.Y, 0.f);
	const FVector SlowedVelocity = FlatVelocity.GetSafeNormal() * FlatVelocity.Size() * PostDeflectSpeedMultiplier;

	ProjectileMovement->SetActive(false);
	ProjectileMovement->SetActive(true);
	ProjectileMovement->Velocity = SlowedVelocity;
	ProjectileMovement->UpdateComponentVelocity();

	UE_LOG(LogTemp, Log, TEXT("CannonBall deflected — new velocity: %s"), *SlowedVelocity.ToString());
}