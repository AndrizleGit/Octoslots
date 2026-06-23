// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgrades/InRunUpgradeData.h"
#include "Upgrades/JokerData.h"
#include "InRunUpgradeManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRunUpgradeSelected, UInRunUpgradeData*, SelectedUpgrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJokerLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJokerSelected, UJokerData*, SelectedJoker);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UInRunUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInRunUpgradeManagerComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InRunUpgrades")
	TArray<TObjectPtr<UInRunUpgradeData>> AllPossibleUpgrades;
	
	// --- jokers ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
	TArray<TObjectPtr<UJokerData>> AllPossibleJokers;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
	int32 JokerLevelInterval = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
	int32 MaxJokers = 2;
	
	// events for widget
	UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
	FOnInRunUpgradeSelected OnUpgradeSelected;
	
	UPROPERTY(BlueprintAssignable, Category = "Jokers")
	FOnJokerLevelUp OnJokerLevelUp;
	
	UPROPERTY(BlueprintAssignable, Category = "Jokers")
	FOnJokerSelected OnJokerSelected;
	
	// function for widget
	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void SelectUpgrade(UInRunUpgradeData* Upgrade);

	UFUNCTION(BlueprintCallable, Category = "Jokers")
	void SelectJoker(UJokerData* Joker);
	
	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void CheckForLevelUp();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	TArray<UInRunUpgradeData*> GetCurrentUpgradeChoices() const { return CurrentChoices; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Jokers")
	TArray<UJokerData*> GetCurrentJokerChoices() const { return CurrentJokerChoices; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void ResetForNewRun();

	// Snapshots the player's current per-run stats as the baseline that ResetForNewRun
	// restores. Call once after meta-progression and base-stat init are complete and
	// before any in-run upgrade or slot buff is applied (see AOctopusCharacter::BeginPlay).
	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void CaptureBaseline();

	UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
	void SetRunActive(bool bActive) { bIsRunActive = bActive; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
	bool IsRunActive() const { return bIsRunActive; }
	
protected:
	virtual void BeginPlay() override;

private:
	void RollNewChoices();
	void RollNewJokerChoices();
	void ApplyUpgrade(UInRunUpgradeData* Upgrade);
	void ApplyStatChange(EInRunUpgradeStat Stat, float Value);
	
	bool bIsRunActive = true;
	// Guards against re-entry: writing the Experience attribute re-fires the attribute
	// change delegate, which calls CheckForLevelUp again synchronously.
	bool bIsProcessingLevelUp = false;
	int32 CurrentLevel = 0;

	// Per-run stat baseline (base defaults + meta-progression), captured once at spawn so
	// ResetForNewRun can return these stats to their start-of-run values without touching
	// meta-progression. Only meaningful once bBaselineCaptured is true.
	bool bBaselineCaptured = false;
	float BaselineMaxHealth = 0.f;
	float BaselineWalkSpeed = 0.f;
	float BaselineAttackSpeed = 0.f;
	float BaselineAttackDamage = 0.f;
	float BaselineConeMaxDistance = 0.f;

	UPROPERTY()
	TArray<UInRunUpgradeData*> CurrentChoices;

	UPROPERTY()
	TArray<UJokerData*> CurrentJokerChoices;
	
	UPROPERTY()
	TMap<UInRunUpgradeData*, int32> PickedCounts;
	
	UPROPERTY()
	TArray<UJokerData*> AcquiredJokers;

	class UBasicAttributeSet* GetPlayerAttributes() const;
};

