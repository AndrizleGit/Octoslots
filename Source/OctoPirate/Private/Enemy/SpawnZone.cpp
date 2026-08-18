#include "Enemy/SpawnZone.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Components/BoxComponent.h"

ASpawnZone::ASpawnZone()
{
    PrimaryActorTick.bCanEverTick = false;

    ZoneBounds = CreateDefaultSubobject<UBoxComponent>("ZoneBounds");
    RootComponent = ZoneBounds;
    ZoneBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ZoneBounds->SetBoxExtent(FVector(1000.f, 1000.f, 500.f));
}

void ASpawnZone::Unlock()
{
    bIsUnlocked = true;
    UE_LOG(LogTemp, Log, TEXT("SpawnZone %s unlocked"), *GetName());
}

void ASpawnZone::Lock()
{
    bIsUnlocked = false;
    UE_LOG(LogTemp, Log, TEXT("SpawnZone %s locked"), *GetName());
}

void ASpawnZone::RegisterSpawn()
{
    EnemiesSpawnedCount++;
    if (EnemiesSpawnedCount >= SpawnLimit)
    {
        StartCooldown();
    }
}

void ASpawnZone::StartCooldown()
{
    bIsOnCooldown = true;
    EnemiesSpawnedCount = 0;
    UE_LOG(LogTemp, Log, TEXT("SpawnZone %s on cooldown for %.1f seconds"), *GetName(), CooldownDuration);

    GetWorldTimerManager().SetTimer(CooldownTimer, this, &ASpawnZone::ResetCooldown, CooldownDuration, false);
}

void ASpawnZone::ResetCooldown()
{
    bIsOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("SpawnZone %s cooldown reset — available again"), *GetName());
}

FVector ASpawnZone::GetRandomPointInZone() const
{
    const FVector Extent = ZoneBounds->GetScaledBoxExtent();
    const FVector LocalRandom = FVector(
        FMath::RandRange(-Extent.X, Extent.X),
        FMath::RandRange(-Extent.Y, Extent.Y),
        0.f
    );
    return ZoneBounds->GetComponentTransform().TransformPosition(LocalRandom);
}

FVector ASpawnZone::GetZoneCenter() const
{
    return ZoneBounds->GetComponentLocation();
}

bool ASpawnZone::IsPointInZone(const FVector& Point) const
{
    if (!ZoneBounds) return false;
    const FVector LocalPoint = ZoneBounds->GetComponentTransform().InverseTransformPosition(Point);
    const FVector Extent = ZoneBounds->GetScaledBoxExtent();
    return FMath::Abs(LocalPoint.X) <= Extent.X
        && FMath::Abs(LocalPoint.Y) <= Extent.Y
        && FMath::Abs(LocalPoint.Z) <= Extent.Z;
}

void ASpawnZone::TriggerBossFightLockdown(UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World) return;

    TArray<AActor*> AllZones;
    UGameplayStatics::GetAllActorsOfClass(World, ASpawnZone::StaticClass(), AllZones);
    for (AActor* ZoneActor : AllZones)
    {
        if (ASpawnZone* Zone = Cast<ASpawnZone>(ZoneActor))
        {
            if (!Zone->bIsBossRoomSpawner) Zone->Lock();
        }
    }

    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(World, ABaseEnemyCharacter::StaticClass(), AllEnemies);
    for (AActor* Enemy : AllEnemies)
    {
        if (Enemy) Enemy->Destroy();
    }
}