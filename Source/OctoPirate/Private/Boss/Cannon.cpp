#include "Boss/Cannon.h"
#include "Boss/Mortar.h"
#include "Projectiles/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Boss/BossMonkey.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"

ACannon::ACannon()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(300.f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    TriggerSphere->SetGenerateOverlapEvents(true);
}

void ACannon::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ACannon::OnOverlapBegin);

    // spawn trigger radius decal on the floor
    if (TriggerDecalMaterial)
    {
        TriggerDecal = UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(),
            TriggerDecalMaterial,
            FVector(TriggerRadius, TriggerRadius, TriggerRadius),
            GetActorLocation() + FVector(0.f, 0.f, 10.f),
            FRotator(-90.f, 0.f, 0.f)
        );
    }

    // debug trigger radius
    DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 16, FColor::Green, true);
}

void ACannon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    if (Cast<AOctopusCharacter>(OtherActor)) return;
    if (bHasFired) return;

    ABaseProjectile* Projectile = Cast<ABaseProjectile>(OtherActor);
    if (!Projectile) return;

    if (Projectile->WasDeflected())
    {
        // deflected cannonball (trigger cannon and destroy ball)
        TriggerCannon();
        Projectile->Destroy();
    }
    else
    {
        // non-deflected cannonball (small explosion VFX but no effect)
        if (FireVFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), FireVFX, Projectile->GetActorLocation());
        }
        Projectile->Destroy();
    }
}

void ACannon::TriggerCannon()
{
    if (bHasFired) return;

    // hide trigger decal
    if (TriggerDecal)
    {
        TriggerDecal->SetVisibility(false);
    }

    // fire after short delay
    GetWorldTimerManager().SetTimer(FireDelayTimer, this, &ACannon::Fire, FireDelay, false);
}

void ACannon::Fire()
{
    if (!LinkedMortar || LinkedMortar->IsDestroyed()) return;

    if (FireVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireVFX, GetActorLocation());
    }

    LinkedMortar->DestroyMortar();
    bHasFired = true;

    // notify boss monkey to check win condition
    if (BossMonkey)
    {
        BossMonkey->CheckAllMortarsDestroyed();
    }

    UE_LOG(LogTemp, Log, TEXT("Cannon %s fired — destroyed mortar %s"),*GetName(), *LinkedMortar->GetName());
}