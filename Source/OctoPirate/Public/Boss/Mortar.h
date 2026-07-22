#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mortar.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UDecalComponent;

UCLASS()
class OCTOPIRATE_API AMortar : public AActor
{
    GENERATED_BODY()

public:
    AMortar();

protected:
    virtual void BeginPlay() override;

public:
    // --- Config ---

    // time between each shot cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float FireInterval = 6.f;

    // delay before first shot (to stagger mortars)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float InitialDelay = 0.f;

    // time between marking the target and the bomb hitting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float WarningDuration = 2.f;

    // height the mortar projectile flies up before the bomb falls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float MortarProjectileHeight = 2000.f;

    // explosion radius on impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float ExplosionRadius = 300.f;

    // damage dealt to player on impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float ExplosionDamage = 20.f;

    // particle played when mortar fires upward (assign in BP_Mortar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar|VFX")
    TObjectPtr<UNiagaraSystem> FireVFX;

    // particle played on bomb impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar|VFX")
    TObjectPtr<UNiagaraSystem> ExplosionVFX;

    // particle played when mortar is destroyed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar|VFX")
    TObjectPtr<UNiagaraSystem> DestructionVFX;

    // material used for the warning decal on the floor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar|VFX")
    TObjectPtr<UMaterialInterface> WarningDecalMaterial;
    
    // size of the warning decal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mortar")
    float WarningDecalSize = 300.f;

    // destroys this mortar (called by ACannon when it fires)
    UFUNCTION(BlueprintCallable, Category = "Mortar")
    void DestroyMortar();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mortar")
    bool IsDestroyed() const { return bIsDestroyed; }

private:
    bool bIsDestroyed = false;

    FTimerHandle FireTimerHandle;
    FTimerHandle WarningTimerHandle;

    // cached target location for current shot cycle
    FVector CurrentTargetLocation = FVector::ZeroVector;

    // active warning decal — spawned at target, removed on impact
    UPROPERTY()
    TObjectPtr<UDecalComponent> ActiveWarningDecal;

    void StartFiring();
    void FireCycle();
    void SpawnWarningDecal(const FVector& TargetLocation);
    void Impact();
};