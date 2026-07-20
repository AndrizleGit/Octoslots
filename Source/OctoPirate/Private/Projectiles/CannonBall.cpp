#include "Projectiles/CannonBall.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACannonBall::ACannonBall()
{
	// cannonball specific defaults
	bCanBeDeflected = true;
	bDealDamageOnHit = true;
	ProjectileDamage = 25.f;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.3f; // slight arc
}

void ACannonBall::OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector)
{
	if (!bCanBeDeflected || bIsDeflected) return;

	bIsDeflected = true;
	SetOwner(Deflector);
	SetInstigator(Cast<APawn>(Deflector));
	CollisionSphere->IgnoreActorWhenMoving(Deflector, true);

	// disable collision with pawns after deflection so it cant hit enemies
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// apply half speed after deflection
	const FVector SlowedVelocity = ReflectedVelocity * PostDeflectSpeedMultiplier;
	ProjectileMovement->Velocity = SlowedVelocity;
	ProjectileMovement->UpdateComponentVelocity();

	UE_LOG(LogTemp, Log, TEXT("CannonBall deflected — new velocity: %s"), *SlowedVelocity.ToString());
}