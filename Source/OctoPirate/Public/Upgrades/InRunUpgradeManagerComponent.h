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

class UAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UInRunUpgradeManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UInRunUpgradeManagerComponent();
    
    // pool of stat upgrades offered on normal level ups
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InRunUpgrades")
    TArray<TObjectPtr<UInRunUpgradeData>> AllPossibleUpgrades;
    
    // pool of jokers offered every JokerLevelInterval levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
    TArray<TObjectPtr<UJokerData>> AllPossibleJokers;
    
    // how often a joker screen appears instead of a normal upgrade screen
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
    int32 JokerLevelInterval = 10;

    // max jokers the player can hold per run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers")
    int32 MaxJokers = 2;
    
    // --- Events (bind in widget Blueprint) ---
    UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
    FOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "InRunUpgrades")
    FOnInRunUpgradeSelected OnUpgradeSelected;
    
    UPROPERTY(BlueprintAssignable, Category = "Jokers")
    FOnJokerLevelUp OnJokerLevelUp;
    
    UPROPERTY(BlueprintAssignable, Category = "Jokers")
    FOnJokerSelected OnJokerSelected;
    
    // --- Widget API ---
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

    // call once after meta-progression and base stats are set (see AOctopusCharacter::BeginPlay)
    UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
    void CaptureBaseline();

    UFUNCTION(BlueprintCallable, Category = "InRunUpgrades")
    void SetRunActive(bool bActive) { bIsRunActive = bActive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InRunUpgrades")
    bool IsRunActive() const { return bIsRunActive; }
    
protected:
    virtual void BeginPlay() override;
    // Ability System Component
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

private:
    // API
    void RollNewChoices();
    void RollNewJokerChoices();
    void ApplyUpgrade(UInRunUpgradeData* Upgrade);
    void ApplyStatChange(EInRunUpgradeStat Stat, float Value);
    
    bool bIsRunActive = true;

    // guards against re-entry when SetExperience fires the attribute delegate synchronously
    bool bIsProcessingLevelUp = false;
    int32 CurrentLevel = 0;

    // baseline stats captured at run start — restored by ResetForNewRun
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
    
    // jokers acquired this run — excluded from future rolls
    UPROPERTY()
    TArray<UJokerData*> AcquiredJokers;

    class UBasicAttributeSet* GetPlayerAttributes() const;
};