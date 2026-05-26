#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InRunUpgradeData.generated.h"

UENUM(BlueprintType)
enum class EInRunUpgradeStat : uint8
{
	MaxHealth       UMETA(DisplayName = "Max Health"),
	MovementSpeed   UMETA(DisplayName = "Movement Speed"),
	AttackSpeed     UMETA(DisplayName = "Attack Speed"),
	AttackDamage    UMETA(DisplayName = "Attack Damage"),
	AttackRange     UMETA(DisplayName = "Attack Range"),
};

UCLASS(BlueprintType)
class OCTOPIRATE_API UInRunUpgradeData : public UDataAsset
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
	EInRunUpgradeStat AffectedStat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	float UpgradeValue = 10.0f;
};
