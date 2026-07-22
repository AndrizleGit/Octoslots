#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "CrabThrower.generated.h"

UENUM()
enum class ECrabThrowerState : uint8
{
    Chasing,
    DiggingUp,
    Throwing,
    Panicking
};

UCLASS()
class OCTOPIRATE_API ACrabThrower : public ABaseEnemyCharacter
{
    GENERATED_BODY()

public:
    ACrabThrower();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;
    
    virtual void PerformAttack_Implementation() override;

public:
    
    // -- sound --
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> ThrowSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Bomb")
    TSubclassOf<AActor> CarriedBombVisualClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Ranges")
    float ThrowRange = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Ranges")
    float PanicRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Bomb")
    TSubclassOf<class ACrabBomb> BombProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Bomb")
    FName BombThrowSocketName = "BombSocket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Bomb")
    float ThrowArcParam = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Bomb")
    float ThrowCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Animation")
    TObjectPtr<UAnimMontage> DigUpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Animation")
    TObjectPtr<UAnimMontage> ThrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrabThrower|Animation")
    TObjectPtr<UAnimMontage> PanicMontage;

private:
    ECrabThrowerState CurrentState = ECrabThrowerState::Chasing;

    void MoveTowardPlayer();
    
    // Crab spawns with a bomb already attached to its back.
    bool bHasBomb = true;

    void EnterChasing();
    void EnterDiggingUp();
    void EnterThrowing();
    void EnterPanicking();
    void ThrowBomb();

    UPROPERTY()
    TObjectPtr<AActor> CarriedBombActor;

    void AttachCarriedBomb();
    void DetachAndDestroyCarriedBomb();
    
    FTimerHandle ThrowCooldownTimer;
    bool bThrowOnCooldown = false;
    
    UFUNCTION()
    void OnDigUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnThrowMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnPanicMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};