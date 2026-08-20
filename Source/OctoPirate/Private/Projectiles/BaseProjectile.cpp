#include "Projectiles/BaseProjectile.h"

#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Components/MeshComponent.h"
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
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnZ = GetActorLocation().Z;
	CollisionSphere->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);

	if (bSpinMeshWhileTraveling && !SpinMesh)
	{
		// the visual mesh lives on the blueprint, so grab the first one that
		// is not the root - spinning the root would fight the movement component
		TArray<UMeshComponent*> MeshComponents;
		GetComponents<UMeshComponent>(MeshComponents);
		for (UMeshComponent* Mesh : MeshComponents)
		{
			if (Mesh && Mesh != GetRootComponent())
			{
				SpinMesh = Mesh;
				break;
			}
		}
	}
}


void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ProjectileMovement->IsActive())
	{
		LastFrameVelocity = ProjectileMovement->Velocity;

		if (bLockToSpawnHeight)
		{
			ProjectileMovement->Velocity.Z = 0.f;
			FVector LockedLocation = GetActorLocation();
			LockedLocation.Z = SpawnZ;
			SetActorLocation(LockedLocation);
		}

		if (bSpinMeshWhileTraveling && SpinMesh && !SpinRate.IsNearlyZero())
		{
			SpinMesh->AddLocalRotation(SpinRate * DeltaTime);
		}
	}
}

void ABaseProjectile::OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector)
{
	if (!bCanBeDeflected || bIsDeflected) return;

	bIsDeflected = true;
	SetOwner(Deflector);
	SetInstigator(Cast<APawn>(Deflector));
	CollisionSphere->IgnoreActorWhenMoving(Deflector, true);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // stop blocking the player post-deflect
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, ReflectedVelocity, Deflector]()
	{
		ProjectileMovement->SetUpdatedComponent(CollisionSphere);
		ProjectileMovement->Velocity = ComputeDeflectedVelocity(ReflectedVelocity, Deflector);
		ProjectileMovement->SetComponentTickEnabled(true);
		ProjectileMovement->Activate(true);
		ProjectileMovement->UpdateComponentVelocity();
	});
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	AOctopusCharacter* Player = Cast<AOctopusCharacter>(OtherActor);
	if (Player && Player->IsDeflectActive() && bCanBeDeflected)
	{
		const FVector IncomingVelocity = LastFrameVelocity;
		const FVector HitNormal = FVector(Hit.ImpactNormal.X, Hit.ImpactNormal.Y, 0.f).GetSafeNormal();
		const FVector ReflectedVelocity = IncomingVelocity - 2.f * FVector::DotProduct(IncomingVelocity, HitNormal) * HitNormal;
		OnDeflected(ReflectedVelocity, Player);
		return;
	}
	
	HandleImpact(OtherActor, Hit);
}

void ABaseProjectile::HandleImpact(AActor* OtherActor, const FHitResult& Hit)
{
	if (bDealDamageOnHit)
	{
		UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, GetInstigatorController(), this, UDamageType::StaticClass());
		Destroy();
	}
}