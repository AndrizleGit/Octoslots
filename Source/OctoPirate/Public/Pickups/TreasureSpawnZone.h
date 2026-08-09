// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreasureSpawnZone.generated.h"
class UBoxComponent;

UCLASS()
class OCTOPIRATE_API ATreasureSpawnZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATreasureSpawnZone();
	
	// Zone Modifiers
	
	//Determines the access level required for spawn treasure in the Zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TreasureZone")
	int ZoneLevel = 0;
	
	//API
	UFUNCTION(BlueprintCallable, Category = "TreasureZone")
	int GetZoneLevel(){return ZoneLevel;};
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TreasureZone")
	FVector GetRandomPointInZone() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TreasureZone")
	TObjectPtr<UBoxComponent> ZoneBounds;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
