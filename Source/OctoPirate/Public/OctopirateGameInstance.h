// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OctopirateGameInstance.generated.h"

UCLASS()
class OCTOPIRATE_API UOctopirateGameInstance : public UGameInstance
{
	GENERATED_BODY()
    
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta Progression")
	float TotalMetaCoins = 0.0f;
};