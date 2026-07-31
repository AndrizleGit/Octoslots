// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"

#include "TreasureMapComponent.generated.h"


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
	// How Far to Search from randomly picked point for Nav Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Search Extent")
	float SearchExtent = 200.f;
	// How Far From The Border 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map|Distance From Border")
	float BorderClearance = 150.f;
	int32 MaxAttempts = 40;
	
	// --- Public API ---
	UFUNCTION(BlueprintCallable, Category = "Treasure Map")
	void AddCannonFragment();
	UFUNCTION(BlueprintCallable, Category = "Treasure Map")
	void SpawnTreasure();
	UFUNCTION(BlueprintCallable, Category = "Treasure Map")
	int SubmitCannonFragment();
	// -- Treasure Actor -- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Treasure Map")
	TSubclassOf<AActor> TreasureClass;
	int CannonFragment = 0;
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
		
};
