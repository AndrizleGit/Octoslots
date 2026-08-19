#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Shark.generated.h"

UENUM()
enum class ESharkChargeState : uint8
{
    None,
    Telegraphing,
    Charging   
};

UCLASS()
class OCTOPIRATE_API AShark : public ABaseEnemyCharacter
{
    GENERATED_BODY()

public:
    AShark();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeTriggerRange = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeCooldown = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float TelegraphDuration = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeSpeed = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeDistance = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeLaneWidth = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float ChargeDamage = 25.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    TObjectPtr<class UMaterialInterface> LaneDecalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Audio")
    TObjectPtr<class USoundBase> TelegraphSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Audio")
    TObjectPtr<class USoundBase> ChargeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shark|Charge")
    float StuckMovementThreshold = 5.f;
private:
    ESharkChargeState ChargeState = ESharkChargeState::None;

    bool bChargeOnCooldown = false;
    FTimerHandle ChargeCooldownTimer;
    FTimerHandle TelegraphTimer;

    FVector ChargeDirection = FVector::ZeroVector;
    FVector ChargeStartLocation = FVector::ZeroVector;
    bool bHasHitPlayerThisCharge = false;

    UPROPERTY()
    TObjectPtr<class UDecalComponent> ActiveLaneDecal;

    void TryBeginTelegraph(float DistToPlayer);
    void BeginTelegraph();
    void BeginCharge();
    void TickCharge(float DeltaTime);
    void EndCharge();
    void ClearLaneDecal();
    
    FVector LastTickLocation = FVector::ZeroVector;
    
    UPROPERTY()
    TArray<AActor*> IgnoredDuringCharge;
    
    float EffectiveChargeDistance = 0.f;
};