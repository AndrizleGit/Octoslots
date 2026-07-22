#include "Boss/BossMonkey.h"
#include "Boss/Mortar.h"
#include "Projectiles/CannonBall.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABossMonkey::ABossMonkey()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABossMonkey::BeginPlay()
{
    Super::BeginPlay();
}

void ABossMonkey::StartBossFight()
{
    if (bIsDefeated || bFightActive) return;

    bFightActive = true;
    UE_LOG(LogTemp, Log, TEXT("Boss fight started"));

    for (AMortar* Mortar : Mortars)
    {
        if (Mortar) Mortar->StartFiring();
    }
    
    ScheduleNextThrow();
}

void ABossMonkey::StopBossFight()
{
    bFightActive = false;
    GetWorldTimerManager().ClearTimer(ThrowTimerHandle);
}

void ABossMonkey::ScheduleNextThrow()
{
    if (!bFightActive || bIsDefeated) return;

    // pick a random interval between min and max
    const float NextThrowTime = FMath::RandRange(MinThrowInterval, MaxThrowInterval);

    GetWorldTimerManager().SetTimer(ThrowTimerHandle,this,&ABossMonkey::ThrowCannonBall,NextThrowTime,false);
}

void ABossMonkey::ThrowCannonBall()
{
    if (!bFightActive || bIsDefeated || !CannonBallClass) return;

    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, ThrowSpawnHeight);
    const FVector TargetLocation = FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, SpawnLocation.Z);
    const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
    const FRotator SpawnRotation = Direction.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    ACannonBall* Ball = GetWorld()->SpawnActor<ACannonBall>(
        CannonBallClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (Ball)
    {
        Ball->ProjectileMovement->Velocity = Direction * Ball->ProjectileMovement->InitialSpeed;
        Ball->ProjectileMovement->UpdateComponentVelocity();
        UE_LOG(LogTemp, Log, TEXT("Boss threw cannonball at player"));
    }

    OnThrowAnimation();
    ScheduleNextThrow();
}

void ABossMonkey::CheckAllMortarsDestroyed()
{
    if (bIsDefeated) return;

    for (AMortar* Mortar : Mortars)
    {
        if (Mortar && !Mortar->IsDestroyed())
            return; // at least one mortar still alive
    }

    // all mortars destroyed (boss defeated)
    bIsDefeated = true;
    StopBossFight();

    UE_LOG(LogTemp, Log, TEXT("All mortars destroyed — boss defeated!"));

    // play defeat animation placeholder
    OnDefeatedAnimation();

    // broadcast event for UI/cutscene
    OnBossDefeated.Broadcast();
}