#include "Boss/Mortar.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/BaseCharacter.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"

AMortar::AMortar()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMortar::BeginPlay()
{
    Super::BeginPlay();
}

void AMortar::StartFiring()
{
    if (bIsDestroyed) return;

    // wait for initial delay then start the repeating fire cycle
    GetWorldTimerManager().SetTimer(FireTimerHandle,this,&AMortar::FireCycle,InitialDelay > 0.f ? InitialDelay : FireInterval,false);
}

void AMortar::FireCycle()
{
    if (bIsDestroyed) return;

    // get current player position as target
    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    CurrentTargetLocation = Player->GetActorLocation();

    // play fire VFX shooting upward from the mortar
    if (FireVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),FireVFX,GetActorLocation(),FRotator(90.f, 0.f, 0.f)); // pointing upward
    }
    
    ABaseCharacter::PlaySFX(this, FireSound, GetActorLocation());
    
    // debug explosion radius
    DrawDebugSphere(GetWorld(), CurrentTargetLocation, ExplosionRadius, 16, FColor::Red, false, WarningDuration);
    
    // show warning decal at target location
    SpawnWarningDecal(CurrentTargetLocation);

    // schedule impact after WarningDuration
    GetWorldTimerManager().SetTimer(WarningTimerHandle,this,&AMortar::Impact,WarningDuration,false);

    // schedule next fire cycle
    GetWorldTimerManager().SetTimer(FireTimerHandle,this,&AMortar::FireCycle,FireInterval,false);
}

void AMortar::SpawnWarningDecal(const FVector& TargetLocation)
{
    if (!WarningDecalMaterial) return;

    // remove previous decal if still active
    if (ActiveWarningDecal)
    {
        ActiveWarningDecal->DestroyComponent();
        ActiveWarningDecal = nullptr;
    }

    // spawn decal on the floor pointing downward
    ActiveWarningDecal = UGameplayStatics::SpawnDecalAtLocation(
        GetWorld(),
        WarningDecalMaterial,
        FVector(WarningDecalSize, WarningDecalSize, WarningDecalSize),
        TargetLocation + FVector(0.f, 0.f, 10.f),
        FRotator(-90.f, 0.f, 0.f)
    );
}

void AMortar::Impact()
{
    if (bIsDestroyed) return;

    if (ActiveWarningDecal)
    {
        ActiveWarningDecal->DestroyComponent();
        ActiveWarningDecal = nullptr;
    }

    DrawDebugSphere(GetWorld(), CurrentTargetLocation, ExplosionRadius, 16, FColor::Red, false, 2.f);

    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, CurrentTargetLocation);
    }
    
    ABaseCharacter::PlaySFX(this, ImpactSound, CurrentTargetLocation);

    // manually check distance to player before applying damage
    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player)
    {
        const float DistToPlayer = FVector::Dist2D(Player->GetActorLocation(), CurrentTargetLocation);
        if (DistToPlayer <= ExplosionRadius)
        {
            UGameplayStatics::ApplyDamage(Player, ExplosionDamage, GetInstigatorController(), this, UDamageType::StaticClass());
        }
    }
}

void AMortar::DestroyMortar()
{
    if (bIsDestroyed) return;

    bIsDestroyed = true;

    // stop all timers
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
    GetWorldTimerManager().ClearTimer(WarningTimerHandle);

    // remove any active warning decal
    if (ActiveWarningDecal)
    {
        ActiveWarningDecal->DestroyComponent();
        ActiveWarningDecal = nullptr;
    }

    // play destruction VFX
    if (DestructionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            DestructionVFX,
            GetActorLocation()
        );
    }
    
    ABaseCharacter::PlaySFX(this, DestructionSound, GetActorLocation());

    UE_LOG(LogTemp, Log, TEXT("Mortar %s destroyed"), *GetName());

    // hide the mortar mesh
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}