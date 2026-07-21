#include "Projectiles/CrabBomb.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ACrabBomb::ACrabBomb()
{
    ProjectileMovement->ProjectileGravityScale = 1.f;
    bLockToSpawnHeight = false;
}

void ACrabBomb::BeginPlay()
{
    Super::BeginPlay();

    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionSphere->SetGenerateOverlapEvents(true);
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACrabBomb::OnPawnOverlap);

    if (AActor* OwningCrab = GetOwner())
    {
        CollisionSphere->IgnoreActorWhenMoving(OwningCrab, true);
    }
}

void ACrabBomb::OnPawnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    if (AOctopusCharacter* Player = Cast<AOctopusCharacter>(OtherActor))
    {
        if (Player->IsDeflectActive() && bCanBeDeflected && !bIsDeflected)
        {
            const FVector IncomingVelocity = ProjectileMovement->Velocity;
            const FVector HitNormal = !SweepResult.ImpactNormal.IsNearlyZero()
                ? FVector(SweepResult.ImpactNormal.X, SweepResult.ImpactNormal.Y, 0.f).GetSafeNormal()
                : -IncomingVelocity.GetSafeNormal();
            const FVector ReflectedVelocity = IncomingVelocity - 2.f * FVector::DotProduct(IncomingVelocity, HitNormal) * HitNormal;
            OnDeflected(ReflectedVelocity, Player);
        }
        else if (!bIsDeflected)
        {
            Explode();
        }
        return;
    }

    if (bIsDeflected && Cast<ABaseEnemyCharacter>(OtherActor))
    {
        Explode();
    }
}

void ACrabBomb::LaunchAtTarget(const FVector& TargetLocation, float ArcParam)
{
    FVector LaunchVelocity;
    const bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
        this, LaunchVelocity, GetActorLocation(), TargetLocation, 0.f, ArcParam);

    if (bSuccess)
    {
        ProjectileMovement->Velocity = LaunchVelocity;
        ProjectileMovement->UpdateComponentVelocity();
    }
}

void ACrabBomb::OnDeflected_Implementation(const FVector& ReflectedVelocity, AActor* Deflector)
{
    Super::OnDeflected_Implementation(ReflectedVelocity, Deflector);
}

FVector ACrabBomb::ComputeDeflectedVelocity(const FVector& ReflectedVelocity, AActor* Deflector) const
{
    const FVector FlatDirection = FVector(ReflectedVelocity.X, ReflectedVelocity.Y, 0.f).GetSafeNormal();
    const FVector TargetLocation = GetActorLocation() + FlatDirection * DeflectThrowDistance;

    FVector LaunchVelocity;
    const bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
        this, LaunchVelocity, GetActorLocation(), TargetLocation, 0.f, DeflectArcParam);

    return bSuccess ? LaunchVelocity : ReflectedVelocity;
}

void ACrabBomb::HandleImpact(AActor* OtherActor, const FHitResult& Hit)
{
    Explode();
}

void ACrabBomb::Explode()
{
    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
    }

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), ExplosionRadius, ObjectTypes, nullptr, { this, GetOwner() }, OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;

        const bool bIsPlayer = Actor->IsA<AOctopusCharacter>();
        const bool bIsEnemy = Actor->IsA<ABaseEnemyCharacter>();

        if (bIsPlayer || (bIsEnemy && bIsDeflected))
        {
            UGameplayStatics::ApplyDamage(Actor, ProjectileDamage, GetInstigatorController(), this, UDamageType::StaticClass());
        }
    }

    Destroy();
}