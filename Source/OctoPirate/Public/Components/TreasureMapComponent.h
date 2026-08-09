// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"

#include "TreasureMapComponent.generated.h"

class ATreasureSpawnZone;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UTreasureMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTreasureMapComponent();
	// -- Configs --
	// -- Treasure Digging Config -- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Digging Time")
	float DiggingTime = 10.f;
	// -- Treasure Spawning Config --
	// How Far does the Treasure Spawn from Player - Default 15m
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Distance From Player")
	float Distance = 1500.f;
	// How Far to Search from randomly picked point for Nav Mesh (X,Y,Z)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Search Extent")
	FVector SearchExtent = FVector(500.f,500.f,500.f);
	// How Far From The Border 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Distance From Border")
	float BorderClearance = 150.f;
	int32 MaxAttempts = 40;
	
	// --- Public API ---
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	void AddCannonAmmo();
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	void SpawnTreasure();
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	void SpawnTreasureInZone();
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	int SubmitCannonAmmo();
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	int GetTreasureLevel() const{return TreasureLevel;};
	//Set if NewLevel > Current , Current = NewLevel
	UFUNCTION(BlueprintCallable, Category = "TreasureMap")
	void SetTreasureLevel(int Level);
	// -- Treasure Actor -- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TreasureMap")
	TSubclassOf<AActor> TreasureClass;
	// -- Treasure Map Configs
	
	int CannonFragment = 0;
	//Determines which areas treasure will be spawning at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map")
	int TreasureLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TreasureMap")
	TArray<TObjectPtr<ATreasureSpawnZone>> TreasureSpawnZones;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	
	UPROPERTY()
	AOctopusCharacter* PlayerCharacter = nullptr;
	
	
	bool IsPointAwayFromNavMeshBorder(UNavigationSystemV1* NavSys, const FVector& Point, float ClearanceRadius, int32 NumSamples = 8);
	FVector GetSpawnLocation(ATreasureSpawnZone* TreasureSpawnZone) const;
	ATreasureSpawnZone*  GetClosestTreasureSpawnZone() const;
	void findAllTreasureSpawnZones();
};
