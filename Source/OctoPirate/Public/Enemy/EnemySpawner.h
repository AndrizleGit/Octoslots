#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemySpawnData.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ABaseEnemyCharacter;

UCLASS()
class OCTOPIRATE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
    
public: 
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

public:
	// Weighted list of enemy types that can spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<TObjectPtr<UEnemySpawnData>> SpawnPool;
    
	// How often a spawn cycle runs in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnCycleInterval = 2.f;
    
	// Hard cap on simultaneous enemies alive at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxEnemies = 30;
    
	// Horizontal distance from player at which enemies spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnDistance = 1800.f;
    
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetSpawningEnabled(bool bEnabled);
    
	// Spawn points more than this many units below the player are rejected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float StairHeightTolerance = 200.f;
    
private:
	void SpawnCycle();
	UEnemySpawnData* PickWeightedEnemy(float DifficultyCoefficient) const;
	FVector GetSpawnLocationOutsideViewport() const;
    
	FTimerHandle SpawnTimerHandle;
    
	UPROPERTY()
	TArray<ABaseEnemyCharacter*> ActiveEnemies;
};