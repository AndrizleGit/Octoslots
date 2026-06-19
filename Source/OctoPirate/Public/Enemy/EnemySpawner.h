// Fill out your copyright notice in the Description page of Project Settings.

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<TObjectPtr<UEnemySpawnData>> SpawnPool;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnCycleInterval = 2.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxEnemies = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnDistance = 1800.f;
	
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetSpawningEnabled(bool bEnabled);
	
private:
	void SpawnCycle();
	UEnemySpawnData* PickWeightedEnemy(float DifficultyCoefficient) const;
	FVector GetSpawnLocationOutsideViewport() const;
	
	FTimerHandle SpawnTimerHandle;
	
	UPROPERTY()
	TArray<ABaseEnemyCharacter*> ActiveEnemies;
};
