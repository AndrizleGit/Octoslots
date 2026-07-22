#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy/EnemySpawnData.h"
#include "SpawnZone.generated.h"

class UBoxComponent;

UCLASS()
class OCTOPIRATE_API ASpawnZone : public AActor
{
    GENERATED_BODY()

public:
    ASpawnZone();

    // enemy pool for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnZone")
    TArray<TObjectPtr<UEnemySpawnData>> ZoneSpawnPool;

    // whether this zone is currently available to spawn enemies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnZone")
    bool bIsUnlocked = false;

    // how many enemies this zone spawns before going on cooldown
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnZone")
    int32 SpawnLimit = 20;
    
    // level of this zone — used to unlock groups of zones together
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnZone")
    int32 ZoneLevel = 1;

    // seconds before this zone resets after hitting SpawnLimit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnZone")
    float CooldownDuration = 30.f;

    // unlocks this zone (call from Blueprint or C++ when player reaches this area)
    UFUNCTION(BlueprintCallable, Category = "SpawnZone")
    void Unlock();

    // locks this zone (call to prevent spawning)
    UFUNCTION(BlueprintCallable, Category = "SpawnZone")
    void Lock();

    // returns true if zone is unlocked and not on cooldown
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnZone")
    bool IsAvailable() const { return bIsUnlocked && !bIsOnCooldown; }

    // called by the spawner each time an enemy is spawned from this zone
    void RegisterSpawn();

    // returns a random point inside the zone bounds
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnZone")
    FVector GetRandomPointInZone() const;

    // returns the center of the zone for distance checks
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnZone")
    FVector GetZoneCenter() const;

    // checks if a world point is inside the zone box
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpawnZone")
    bool IsPointInZone(const FVector& Point) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnZone")
    TObjectPtr<UBoxComponent> ZoneBounds;

private:
    int32 EnemiesSpawnedCount = 0;
    bool bIsOnCooldown = false;
    FTimerHandle CooldownTimer;

    void StartCooldown();
    void ResetCooldown();
};