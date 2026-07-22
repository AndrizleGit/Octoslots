#include "Projectiles/BaseProjectile.h"

#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(15.f);
    
	// collision setup
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->SetGenerateOverlapEvents(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.f; // zero bounciness so it doesn't actually bounce on its own
	ProjectileMovement->bBounceAngleAffectsFriction = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionSphere->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// store velocity before any hit processing
	if (ProjectileMovement->IsActive())
	{
		LastFrameVelocity = ProjectileMovement->Velocity;
	}
}

void ABaseProjectile::OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector)
{
	if (!bCanBeDeflected || bIsDeflected) return;

	bIsDeflected = true;
	SetOwner(Deflector);
	SetInstigator(Cast<APawn>(Deflector));
	CollisionSphere->IgnoreActorWhenMoving(Deflector, true);
    
	// with bShouldBounce=true the movement component stays active after hit
	// so we can directly override the velocity
	ProjectileMovement->Velocity = ReflectedVelocity;
	ProjectileMovement->UpdateComponentVelocity();
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	AOctopusCharacter* Player = Cast<AOctopusCharacter>(OtherActor);
	if (Player && Player->IsDeflectActive() && bCanBeDeflected)
	{
		const FVector IncomingVelocity = LastFrameVelocity;
		const FVector HitNormal = FVector(Hit.ImpactNormal);
		const FVector ReflectedVelocity = IncomingVelocity - 2.f * 
			FVector::DotProduct(IncomingVelocity, HitNormal) * HitNormal;
		OnDeflected(ReflectedVelocity, Player);
		return;
	}

	if (bDealDamageOnHit)
	{
		UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, 
			GetInstigatorController(), this, UDamageType::StaticClass());
		Destroy();
	}
}