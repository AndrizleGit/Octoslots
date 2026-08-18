#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgrades/UpgradeData.h"
#include "UpgradeManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradePurchased, UUpgradeData*, Upgrade, int32, NewLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllUpgradesRefunded);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUpgradeManagerComponent();

	/// Save Slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	FString SaveSlotName = "MetaProgressionSave";
	
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void SaveUpgrades();

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void LoadUpgrades();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	TArray<TObjectPtr<UUpgradeData>> AvailableUpgrades;
	
	// --- booleen ---
	UPROPERTY(BlueprintReadWrite, Category = "Config")
    bool bIsMetaProgressionMode = false;

	/// Die Events kannst du an das widget binden
	UPROPERTY(BlueprintAssignable, Category = "Upgrades")
	FOnUpgradePurchased OnUpgradePurchased;
	
	UPROPERTY(BlueprintAssignable, Category = "Upgrades")
	FOnAllUpgradesRefunded OnAllUpgradesRefunded;
    
	// Die Functions kannst du vom widget callen
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	bool PurchaseUpgrade(UUpgradeData* Upgrade);
	
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
    bool SellSingleUpgrade(UUpgradeData* Upgrade);

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void RefundAllUpgrades();

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void SelectUpgrade(UUpgradeData* Upgrade);
	
	// Alles getter fuer sachen die man vielleicht wissen muss
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	int32 GetUpgradeLevel(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	float GetCurrentStatValue(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	float GetNextStatValue(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	int32 GetUpgradeCost(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	bool CanAffordUpgrade(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	bool IsUpgradeMaxLevel(UUpgradeData* Upgrade) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	UUpgradeData* GetSelectedUpgrade() const { return SelectedUpgrade; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	int32 GetTotalCoinsSpent() const { return TotalCoinsSpent; }
	
protected:
	virtual void BeginPlay() override;

public:	
	void ApplyUpgrade(UUpgradeData* Upgrade);
	void RemoveUpgrade(UUpgradeData* Upgrade, int32 Levels);
	void ApplyStatChange(EUpgradeStat Stat, float Value);

	UPROPERTY()
	TMap<UUpgradeData*, int32> UpgradeLevels;

	UPROPERTY()
	TObjectPtr<UUpgradeData> SelectedUpgrade;

	int32 TotalCoinsSpent = 0;

	class UBasicAttributeSet* GetPlayerAttributes() const;
};
