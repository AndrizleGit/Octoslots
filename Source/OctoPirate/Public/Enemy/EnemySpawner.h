// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

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
	TSubclassOf<ABaseEnemyCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnInterval = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 MaxEnemies = 20;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnDistance = 1800.f;
	
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetSpawnInterval(float NewInterval);
	
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetSpawningEnabled(bool bEnabled);
	
private:
	void SpawnEnemy();
	FVector GetSpawnLocationOutsideViewport() const;
	
	FTimerHandle SpawnTimerHandle;
	
	UPROPERTY()
	TArray<ABaseEnemyCharacter*> ActiveEnemies;
};
