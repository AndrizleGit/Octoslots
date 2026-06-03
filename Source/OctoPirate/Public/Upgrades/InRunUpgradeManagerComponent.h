// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgrades/InRunUpgradeData.h"
#include "InRunUpgradeManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRunUpgradeSelected, UInRunUpgradeData*, SelectedUpgrade);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UInRunUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInRunUpgradeManagerComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InRunUpgrades")
	TArray<TObjectPtr<UInRunUpgradeData>> AllPossibleUpgrades;
	
	// events for widget
	UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
	FOnInRunUpgradeSelected OnUpgradeSelected;
	
	// function for widget
	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void SelectUpgrade(UInRunUpgradeData* Upgrade);

	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void CheckForLevelUp();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	TArray<UInRunUpgradeData*> GetCurrentUpgradeChoices() const { return CurrentChoices; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void ResetForNewRun();

	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void SetRunActive(bool bActive) { bIsRunActive = bActive; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	bool IsRunActive() const { return bIsRunActive; }
	
protected:
	virtual void BeginPlay() override;

private:
	void RollNewChoices();
	void ApplyUpgrade(UInRunUpgradeData* Upgrade);
	void ApplyStatChange(EInRunUpgradeStat Stat, float Value);

	bool bIsRunActive = true;
	int32 CurrentLevel = 0;

	UPROPERTY()
	TArray<UInRunUpgradeData*> CurrentChoices;

	UPROPERTY()
	TMap<UInRunUpgradeData*, int32> PickedCounts;

	class UBasicAttributeSet* GetPlayerAttributes() const;
};

