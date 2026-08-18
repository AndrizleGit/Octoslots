#include "Enemy/EnemySpawner.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Enemy/DifficultyManager.h"
#include "Enemy/SpawnZone.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    for (ASpawnZone* Zone : SpawnZones)
    {
        if (!Zone) continue;
        if (Zone->ZoneLevel == 1)
            Zone->Unlock();
        else
            Zone->Lock();
    }
    
    if (SpawnZones.Num() > 0)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnCycle, SpawnCycleInterval, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: No SpawnZones assigned!"));
    }
}

void AEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
    if (bEnabled) GetWorldTimerManager().UnPauseTimer(SpawnTimerHandle);
    else GetWorldTimerManager().PauseTimer(SpawnTimerHandle);
}

void AEnemySpawner::SpawnCycle()
{
    // clean up dead enemies
    ActiveEnemies.RemoveAll([](ABaseEnemyCharacter* Enemy)
    {
        return !IsValid(Enemy);
    });

    if (ActiveEnemies.Num() >= MaxEnemies) return;

    // find the closest available zone to the player
    ASpawnZone* ActiveZone = GetClosestAvailableZone();
    if (!ActiveZone) return;

    ADifficultyManager* DifficultyManager = ADifficultyManager::Get(GetWorld());
    const float Coefficient = DifficultyManager ? DifficultyManager->GetDifficultyCoefficient() : 0.f;
    float Budget = DifficultyManager ? DifficultyManager->GetCurrentSpawnBudget() : 4.f;

    while (Budget > 0.f && ActiveEnemies.Num() < MaxEnemies)
    {
        UEnemySpawnData* Choice = PickWeightedEnemyFromPool(ActiveZone->ZoneSpawnPool, Coefficient);
        if (!Choice || !Choice->EnemyClass) break;

        Budget -= Choice->SpawnCost;
        if (Budget < 0.f) break;

        const FVector SpawnLocation = GetSpawnLocationInZone(ActiveZone);
        if (SpawnLocation == FVector::ZeroVector) break;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ABaseEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
            Choice->EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (NewEnemy)
        {
            ActiveEnemies.Add(NewEnemy);
            ActiveZone->RegisterSpawn();

            if (DifficultyManager)
            {
                NewEnemy->ApplyDifficultyScaling(
                    DifficultyManager->GetHealthMultiplier(),
                    DifficultyManager->GetDamageMultiplier());
            }
        }
    }
}

// finds the unlocked, non-cooldown zone closest to the player
ASpawnZone* AEnemySpawner::GetClosestAvailableZone() const
{
    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return nullptr;

    ASpawnZone* Closest = nullptr;
    float ClosestDist = FLT_MAX;

    for (ASpawnZone* Zone : SpawnZones)
    {
        if (!Zone || !Zone->IsAvailable()) continue;

        const float Dist = FVector::Dist(Player->GetActorLocation(), Zone->GetZoneCenter());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Closest = Zone;
        }
    }

    return Closest;
}

// picks a random enemy using inverse-cost weighting
UEnemySpawnData* AEnemySpawner::PickWeightedEnemyFromPool(const TArray<UEnemySpawnData*>& Pool, float DifficultyCoefficient) const
{
    TArray<UEnemySpawnData*> EligiblePool;
    float TotalWeight = 0.f;

    for (UEnemySpawnData* Entry : Pool)
    {
        if (Entry && DifficultyCoefficient >= Entry->MinDifficultyToSpawn)
        {
            EligiblePool.Add(Entry);
            TotalWeight += (1.f / FMath::Max(Entry->SpawnCost, 0.01f));
        }
    }

    if (EligiblePool.Num() == 0) return nullptr;

    float Roll = FMath::FRandRange(0.f, TotalWeight);
    for (UEnemySpawnData* Entry : EligiblePool)
    {
        const float Weight = 1.f / FMath::Max(Entry->SpawnCost, 0.01f);
        if (Roll <= Weight) return Entry;
        Roll -= Weight;
    }

    return EligiblePool.Last();
}

// finds a valid navmesh point inside the zone bounds
FVector AEnemySpawner::GetSpawnLocationInZone(ASpawnZone* Zone) const
{
    if (!Zone) return FVector::ZeroVector;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return FVector::ZeroVector;

    for (int32 Attempt = 0; Attempt < 20; Attempt++)
    {
        FVector CandidateLocation = Zone->GetRandomPointInZone();

        FNavLocation NavLocation;
        if (!NavSys->ProjectPointToNavigation(
            CandidateLocation, NavLocation, FVector(500.f, 500.f, 500.f)))
        {
            continue;
        }

        return NavLocation.Location;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: Failed to find spawn location in zone %s"), *Zone->GetName());
    return FVector::ZeroVector;
}

void AEnemySpawner::UnlockZonesOfLevel(int32 Level)
{
    for (ASpawnZone* Zone : SpawnZones)
    {
        if (Zone && Zone->ZoneLevel == Level)
        {
            Zone->Unlock();
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Unlocked all zones of level %d"), Level);
}

void AEnemySpawner::LockZonesOfLevel(int32 Level)
{
    for (ASpawnZone* Zone : SpawnZones)
    {
        if (Zone && Zone->ZoneLevel == Level)
        {
            Zone->Lock();
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Locked all zones of level %d"), Level);
}