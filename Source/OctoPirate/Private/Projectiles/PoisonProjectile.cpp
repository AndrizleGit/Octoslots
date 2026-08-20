#include "Projectiles/PoisonProjectile.h"

#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"

APoisonProjectile::APoisonProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// This one is the player's own attack: it passes through everything it hits and
	// deals no impact damage, it only applies the poison effect.
	bCanBeDeflected = false;
	bDealDamageOnHit = false;

	CollisionSphere->SetSphereRadius(45.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap); // "Enemy"
	CollisionSphere->SetNotifyRigidBodyCollision(false);
	CollisionSphere->SetGenerateOverlapEvents(true);

	PoisonVFX = CreateDefaultSubobject<UNiagaraComponent>("PoisonVFX");
	PoisonVFX->SetupAttachment(CollisionSphere);

	// Speed is driven every frame in Tick, so the movement component must not clamp it.
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
}

void APoisonProjectile::BeginPlay()
{
	Super::BeginPlay();

	TravelOrigin = GetActorLocation();

	TravelDirection = GetActorForwardVector();
	TravelDirection.Z = 0.f;
	if (!TravelDirection.Normalize())
	{
		TravelDirection = FVector::ForwardVector;
	}

	ProjectileMovement->Velocity = TravelDirection * LaunchSpeed;

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APoisonProjectile::OnPoisonOverlap);

	// Enemies already standing inside the sphere at spawn time never fire a begin-overlap
	// event, and the player attacks at melee range, so catch them explicitly.
	TArray<AActor*> AlreadyOverlapping;
	CollisionSphere->GetOverlappingActors(AlreadyOverlapping);
	for (AActor* Actor : AlreadyOverlapping)
	{
		TryPoisonActor(Actor);
	}
}

void APoisonProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTravelComplete) return;

	const float Travelled = FVector::Dist2D(GetActorLocation(), TravelOrigin);
	if (Travelled >= TravelDistance)
	{
		CompleteTravel();
		return;
	}

	// Ease the speed down from LaunchSpeed to LaunchSpeed * EndSpeedFraction over the
	// travel distance. The exponent keeps it fast for most of the flight and lets it
	// fall off near the end (exponent 1 gives a plain linear slowdown).
	const float Alpha = FMath::Pow(GetTravelProgress(), FMath::Max(SpeedFalloffExponent, KINDA_SMALL_NUMBER));
	const float Speed = FMath::Lerp(LaunchSpeed, LaunchSpeed * EndSpeedFraction, Alpha);

	// A ball that has slowed to a crawl reads as a stuck prop, so retire it as soon as
	// the speed goes low rather than letting it inch out the rest of the distance.
	if (MinSpeedBeforeDespawn > 0.f && Speed <= MinSpeedBeforeDespawn)
	{
		CompleteTravel();
		return;
	}

	ProjectileMovement->Velocity = TravelDirection * Speed;
}

float APoisonProjectile::GetTravelProgress() const
{
	if (TravelDistance <= KINDA_SMALL_NUMBER) return 1.f;
	return FMath::Clamp(FVector::Dist2D(GetActorLocation(), TravelOrigin) / TravelDistance, 0.f, 1.f);
}

void APoisonProjectile::OnPoisonOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryPoisonActor(OtherActor);
}

void APoisonProjectile::TryPoisonActor(AActor* Target)
{
	if (bTravelComplete) return;
	if (!IsValid(Target) || Target == this || Target == GetOwner() || Target == GetInstigator()) return;
	if (PoisonedActors.Contains(Target)) return;

	AOctopusCharacter* Player = Cast<AOctopusCharacter>(GetOwner());
	if (!Player)
	{
		Player = Cast<AOctopusCharacter>(GetInstigator());
	}
	if (!Player) return;

	// Mark it either way — a target that is immune or already poisoned should not be
	// re-tested every frame it stays inside the sphere.
	PoisonedActors.Add(Target);

	if (PoisonEffectMode == EPoisonProjectileEffect::ThreeOfAKind)
	{
		Player->ApplyThreeOfAKindPoisonToActor(Target);
	}
	else
	{
		Player->ApplyPoisonToActor(Target);
	}
}

void APoisonProjectile::CompleteTravel()
{
	if (bTravelComplete) return;
	bTravelComplete = true;

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PoisonVFX)
	{
		// Deactivate (not destroy) so already-spawned particles finish their lifetime.
		PoisonVFX->Deactivate();
	}

	SetLifeSpan(FMath::Max(VFXLingerTime, 0.01f));
}
