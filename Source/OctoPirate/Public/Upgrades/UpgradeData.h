// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeData.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EUpgradeStat : uint8
{
	MaxHealth       UMETA(DisplayName = "Max Health"),
	MovementSpeed   UMETA(DisplayName = "Movement Speed"),
	AttackSpeed     UMETA(DisplayName = "Attack Speed"),
	AttackDamage    UMETA(DisplayName = "Attack Damage"),
};

UCLASS(BlueprintType)
class OCTOPIRATE_API UUpgradeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText UpgradeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText UpgradeDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TObjectPtr<UTexture2D> UpgradeIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	EUpgradeStat AffectedStat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	float ValuePerLevel = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 CostPerLevel = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 MaxLevel = 5;
};