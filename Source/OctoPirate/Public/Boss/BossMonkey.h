#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossMonkey.generated.h"

class ACannonBall;
class AMortar;

// fired when all mortars are destroyed — bind in Blueprint for UI/cutscene/game end
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDefeated);

UCLASS()
class OCTOPIRATE_API ABossMonkey : public AActor
{
    GENERATED_BODY()

public:
    ABossMonkey();

protected:
    virtual void BeginPlay() override;

public:
    // --- Config ---

    // cannonball class to spawn — assign BP_CannonBall here
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossMonkey")
    TSubclassOf<ACannonBall> CannonBallClass;

    // all 3 mortars — assigned in editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossMonkey")
    TArray<TObjectPtr<AMortar>> Mortars;

    // minimum time between cannonball throws
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossMonkey")
    float MinThrowInterval = 3.f;

    // maximum time between cannonball throws
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossMonkey")
    float MaxThrowInterval = 7.f;

    // height offset above actor location where cannonball spawns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossMonkey")
    float ThrowSpawnHeight = 200.f;

    // --- Events ---

    // broadcast when all mortars are destroyed
    UPROPERTY(BlueprintAssignable, Category = "BossMonkey")
    FOnBossDefeated OnBossDefeated;

    // --- API ---

    // call this to start the boss fight (bind to trigger volume overlap in Blueprint)
    UFUNCTION(BlueprintCallable, Category = "BossMonkey")
    void StartBossFight();

    // call this to stop everything (called internally when boss is defeated)
    UFUNCTION(BlueprintCallable, Category = "BossMonkey")
    void StopBossFight();

    // checks if all mortars are destroyed (called after each mortar dies)
    UFUNCTION(BlueprintCallable, Category = "BossMonkey")
    void CheckAllMortarsDestroyed();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BossMonkey")
    bool IsBossDefeated() const { return bIsDefeated; }

    // placeholder (replace with actual throw animation when model is ready)
    UFUNCTION(BlueprintImplementableEvent, Category = "BossMonkey")
    void OnThrowAnimation();

    // placeholder (replace with death animation/sequence when model is ready)
    UFUNCTION(BlueprintImplementableEvent, Category = "BossMonkey")
    void OnDefeatedAnimation();

private:
    bool bIsDefeated = false;
    bool bFightActive = false;

    FTimerHandle ThrowTimerHandle;

    void ThrowCannonBall();
    void ScheduleNextThrow();
};