#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemySpawnData.generated.h"

class ABaseEnemyCharacter;

UCLASS()
class OCTOPIRATE_API UEnemySpawnData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<ABaseEnemyCharacter> EnemyClass;
	
	// How much this enemy "costs" of the spawn budget (The weaker the enemy the cheaper the cost)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpawnCost = 1.f;
	
	// Min difficulty for the enemy to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float MinDifficultyToSpawn = 0.f;
};
