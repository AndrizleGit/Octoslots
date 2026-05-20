// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OctoPirateEnemySpawner.generated.h"

class AOctoPirateEnemyCharacter;

UCLASS()
class OCTOPIRATE_API AOctoPirateEnemySpawner : public AActor
{
	GENERATED_BODY()

public:

	AOctoPirateEnemySpawner();

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category="Spawner")
	TSubclassOf<AOctoPirateEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category="Spawner")
	float SpawnInterval = 3.f;

	UPROPERTY(EditAnywhere, Category="Spawner")
	int32 MaxEnemies = 10;

	UPROPERTY(EditAnywhere, Category="Spawner")
	float MinSpawnDistance = 1500.f;

	UPROPERTY(EditAnywhere, Category="Spawner")
	float MaxSpawnDistance = 3000.f;

	UPROPERTY()
	TArray<TWeakObjectPtr<AOctoPirateEnemyCharacter>> SpawnedEnemies;

	FTimerHandle SpawnTimerHandle;

	UFUNCTION()
	void SpawnEnemy();

	int32 GetLiveEnemyCount() const;
};
