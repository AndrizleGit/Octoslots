#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "SlotMachineTypes.h"
#include "SlotMachineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpinComplete, FSlotResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDopamineChanged, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDopamineEmpty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDebuffStateChanged, bool, bIsDebuffed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpinFailed);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API USlotMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public: 
    USlotMachineComponent();

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Dopamine")
    float DopamineMax = 100.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Dopamine")
    float DopamineDrainPerSecond = 5.f;

    // coins deducted from the player each time they spin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Cost")
    float SpinCost = 10.f;

    // seconds between allowed spins — reset by OnSpinAnimationFinished
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Cost")
    float SpinCooldown = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Buff")
    FSlotBuffConfig BuffConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Debuff")
    float DebuffMovementSpeedPenalty = -200.f;

    // buff strength multiplier applied when auto-spinning via the AutoSpin joker
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Joker")
    float AutoSpinBuffMultiplier = 1.5f;
    
    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
    FOnSpinComplete OnSpinComplete;
    
    UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
    FOnDopamineChanged OnDopamineChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
    FOnDopamineEmpty OnDopamineEmpty;
    
    UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
    FOnDebuffStateChanged OnDebuffStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
    FOnSpinFailed OnSpinFailed;
    
    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Slot Machine")
    void Spin();

    // called from BP_3DSlotMachine after the lever animation finishes to re-enable spinning
    UFUNCTION(BlueprintCallable, Category = "Slot Machine")
    void OnSpinAnimationFinished();
    
    UFUNCTION(BlueprintPure, Category = "Slot Machine")
    float GetDopamineNormalized() const { return DopamineCurrent / DopamineMax; }

    UFUNCTION(BlueprintPure, Category = "Slot Machine")
    bool IsDebuffActive() const { return bDebuffActive; }
    
    UFUNCTION(BlueprintPure, Category = "Slot Machine")
    FSlotResult GetLastResult() const { return LastResult; }

    UFUNCTION(BlueprintPure, Category = "Slot Machine")
    bool CanAffordSpin() const;

    UFUNCTION(BlueprintPure, Category = "Slot Machine")
    bool IsOnCooldown() const { return bSpinOnCooldown; }

    // converts an ESlotSymbol to its display name for use in the UI
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Slot Machine")
    static FText GetSymbolDisplayName(ESlotSymbol Symbol)
    {
        return FText::FromString(UEnum::GetDisplayValueAsText(Symbol).ToString());
    }
    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
private:
    UPROPERTY()
    AOctopusCharacter* PlayerCharacter = nullptr;

    float DopamineCurrent = 100.f;
    bool bDebuffActive = false;
    FSlotResult LastResult;
    bool bSpinOnCooldown = false;

    // set to true during auto-spin so ApplyBuffs can apply the stronger multiplier
    mutable bool bIsAutoSpinning = false;

    // tracks cooldown between auto-spins separately from the animation cooldown
    float AutoSpinCooldownRemaining = 0.0f;

    static FSlotResult RollReels();
    void ApplyBuffs(const FSlotResult& Result) const;
    void RemoveAllBuffs() const;
    void SetDebuffActive(bool bActive);
};