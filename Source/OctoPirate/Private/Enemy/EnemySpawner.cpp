#include "Enemy/EnemySpawner.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Enemy/DifficultyManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    if (SpawnPool.Num() > 0)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnCycle, SpawnCycleInterval, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy Spawner: SpawnPool is empty!"))
    }
}

void AEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
    if (bEnabled) GetWorldTimerManager().UnPauseTimer(SpawnTimerHandle);
    else GetWorldTimerManager().PauseTimer(SpawnTimerHandle);
}

void AEnemySpawner::SpawnCycle()
{
    // Clean up destroyed enemies from the tracking list
    ActiveEnemies.RemoveAll([](ABaseEnemyCharacter* Enemy)
    {
        return !IsValid(Enemy);
    });
    
    if (ActiveEnemies.Num() >= MaxEnemies) return;
    
    ADifficultyManager* DifficultyManager = ADifficultyManager::Get(GetWorld());
    const float Coefficient = DifficultyManager ? DifficultyManager->GetDifficultyCoefficient() : 0.f;
    float Budget = DifficultyManager ? DifficultyManager->GetCurrentSpawnBudget() : 4.f;
    
    // Spend the budget on weighted enemy picks until it runs out or cap is reached
    while (Budget > 0.0f && ActiveEnemies.Num() < MaxEnemies)
    {
        UEnemySpawnData* Choice = PickWeightedEnemy(Coefficient);

        if (!Choice || !Choice->EnemyClass) break;
        
        Budget -= Choice->SpawnCost;
        if (Budget < 0.f) break;
        
        const FVector SpawnLocation = GetSpawnLocationOutsideViewport();
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        ABaseEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
            Choice->EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (NewEnemy)
        {
            ActiveEnemies.Add(NewEnemy);
            
            if (DifficultyManager)
            {
                NewEnemy->ApplyDifficultyScaling(
                    DifficultyManager->GetHealthMultiplier(),
                    DifficultyManager->GetDamageMultiplier());
            }
        }
    }
}

// Picks a random enemy type from the eligible pool using inverse-cost weighting
// Cheaper enemies are picked more often
// Enemies below MinDifficultyToSpawn are excluded
UEnemySpawnData* AEnemySpawner::PickWeightedEnemy(float DifficultyCoefficient) const
{
    TArray<UEnemySpawnData*> EligiblePool;
    float TotalWeight = 0.f;
    
    for (UEnemySpawnData* Entry : SpawnPool)
    {
        if (Entry && DifficultyCoefficient > Entry->MinDifficultyToSpawn)
        {
            EligiblePool.Add(Entry);
            TotalWeight += (1.f / FMath::Max(Entry->SpawnCost, 0.01f));
        }
    }
    
    if (EligiblePool.Num() == 0) return nullptr;
    
    // Weighted random selection
    float Roll = FMath::FRandRange(0.f, TotalWeight);
    for (UEnemySpawnData* Entry : EligiblePool)
    {
        const float Weight = 1.f / FMath::Max(Entry->SpawnCost, 0.01f);
        if (Roll <= Weight) return Entry;
        Roll -= Weight;
    }
    
    return EligiblePool.Last();
}

// Finds a valid navmesh point at SpawnDistance from the player.
// Retries up to 20 times with different angles. Rejects points that are too far
// below the player to avoid spawning under elevated geometry.
FVector AEnemySpawner::GetSpawnLocationOutsideViewport() const
{
    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    const FVector PlayerLocation = Player ? Player->GetActorLocation() : FVector::ZeroVector;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return FVector::ZeroVector;

	for (int32 Attempt = 0; Attempt < 20; Attempt++)
	{
		const float RandomAngle = FMath::RandRange(0.f, 360.f);
		FVector CandidateLocation = PlayerLocation + FVector(
			FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance,
			FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance,
			0.f
		);

		CandidateLocation.Z = PlayerLocation.Z + 200.f;

		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(
			CandidateLocation, NavLocation, FVector(500.f, 500.f, 2000.f)))
		{
			continue; 
		}

		const float HorizontalDist = FVector::Dist2D(NavLocation.Location, PlayerLocation);
		if (HorizontalDist < SpawnDistance * 0.5f)
		{
			continue; 
		}
		
		const float HeightDiff = NavLocation.Location.Z - PlayerLocation.Z;
		if (HeightDiff < -StairHeightTolerance)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Rejecting spawn at Z=%.1f (player Z=%.1f, diff=%.1f)"),
				NavLocation.Location.Z, PlayerLocation.Z, HeightDiff);
			continue;
		}

        // Skip if projection snapped too close to the player
        const float HorizontalDist = FVector::Dist2D(NavLocation.Location, PlayerLocation);
        if (HorizontalDist < SpawnDistance * 0.5f) continue;
        
        // Skip points significantly below the player (under elevated floors)
        const float HeightDiff = NavLocation.Location.Z - PlayerLocation.Z;
        if (HeightDiff < -StairHeightTolerance) continue;

        return NavLocation.Location;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: Failed to find valid spawn location"));
    return FVector::ZeroVector;
}