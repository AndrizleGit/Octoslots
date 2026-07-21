#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemySpawnData.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ABaseEnemyCharacter;
class ASpawnZone;

// Spawns enemies from the zone closest to the player.
// Only unlocked zones are considered. Zones go on cooldown after hitting their SpawnLimit.
UCLASS()
class OCTOPIRATE_API AEnemySpawner : public AActor
{
    GENERATED_BODY()
    
public: 
    AEnemySpawner();

protected:
    virtual void BeginPlay() override;

public:
    // all spawn zones in the level (assign in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TArray<TObjectPtr<ASpawnZone>> SpawnZones;

    // how often a spawn cycle runs in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnCycleInterval = 2.f;

    // hard cap on simultaneous enemies alive at once
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int32 MaxEnemies = 30;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SetSpawningEnabled(bool bEnabled);
    
    // unlocks all zones with the matching ZoneLevel
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void UnlockZonesOfLevel(int32 Level);

    // locks all zones with the matching ZoneLevel
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void LockZonesOfLevel(int32 Level);

private:
    void SpawnCycle();

    // finds the available zone closest to the player
    ASpawnZone* GetClosestAvailableZone() const;

    // picks a weighted enemy from the given pool
    UEnemySpawnData* PickWeightedEnemyFromPool(const TArray<UEnemySpawnData*>& Pool, float DifficultyCoefficient) const;

    // finds a valid navmesh spawn point inside the given zone
    FVector GetSpawnLocationInZone(ASpawnZone* Zone) const;

    FTimerHandle SpawnTimerHandle;

    UPROPERTY()
    TArray<ABaseEnemyCharacter*> ActiveEnemies;
};