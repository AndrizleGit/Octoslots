// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "OctoSlotsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class OCTOPIRATE_API UOctoSlotsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Save")
	TMap<FString, int32> UpgradeLevels;

	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 TotalCoinsSpent = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	float SavedCoins = 0.0f;
};
