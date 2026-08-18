#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DifficultyManager.generated.h"

UCLASS()
class OCTOPIRATE_API ADifficultyManager : public AActor
{
    GENERATED_BODY()
    
public: 
    ADifficultyManager();

protected:
    virtual void BeginPlay() override;

public: 
    virtual void Tick(float DeltaTime) override;

    // --- Difficulty Coefficient ---
    
    // how much coefficient rises per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float CoefficientPerSecond = 0.05f;
    
    // how much player level increases coefficient
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float CoefficientPerPlayerLevel = 0.6f;
    
    // enemy hp multiplier based on coefficient
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float HPScalePerCoefficient = 0.15f;
    
    // enemy damage multiplier based on coefficient
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float DamageScalePerCoefficient = 0.08f;

    // spawn budget per spawn cycle (scales with coefficient)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float BaseSpawnBudget = 4.f;
    
    // how much the spawn budget grows per unit of coefficient
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float SpawnBudgetPerCoefficient = 1.2f;
    
    // --- GETTERS ---
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetDifficultyCoefficient() const { return DifficultyCoefficient; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetHealthMultiplier() const { return 1.f + DifficultyCoefficient * HPScalePerCoefficient; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetDamageMultiplier() const { return 1.f + CoefficientPerPlayerLevel * DamageScalePerCoefficient; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetCurrentSpawnBudget() const { return BaseSpawnBudget + DifficultyCoefficient * SpawnBudgetPerCoefficient; }
    
    // finds the DifficultyManager instance in the current world
    UFUNCTION(BlueprintCallable, Category = "Difficulty")
    static ADifficultyManager* Get(UWorld* World);
    
private:
    float DifficultyCoefficient = 0.f;
    float ElapsedTime = 0.f;
    
    UPROPERTY()
    class AOctopusCharacter* PlayerCharacter;
};