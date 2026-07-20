#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cannon.generated.h"

class UNiagaraSystem;
class UDecalComponent;
class AMortar;

UCLASS()
class OCTOPIRATE_API ACannon : public AActor
{
	GENERATED_BODY()

public:
	ACannon();

protected:
	virtual void BeginPlay() override;

public:
	// the mortar this cannon is linked to and will destroy when fired
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	TObjectPtr<AMortar> LinkedMortar;

	// radius on the floor showing where the player needs to deflect the cannonball
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	float TriggerRadius = 300.f;

	// delay between cannonball hitting trigger and cannon firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	float FireDelay = 0.5f;

	// particle played when cannon fires
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|VFX")
	TObjectPtr<UNiagaraSystem> FireVFX;

	// material for the trigger radius decal on the floor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|VFX")
	TObjectPtr<UMaterialInterface> TriggerDecalMaterial;

	// called when a deflected cannonball enters the trigger radius
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	void TriggerCannon();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cannon")
	bool HasFired() const { return bHasFired; }

private:
	bool bHasFired = false;
	FTimerHandle FireDelayTimer;

	UPROPERTY()
	TObjectPtr<UDecalComponent> TriggerDecal;

	void Fire();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cannon")
	TObjectPtr<class USphereComponent> TriggerSphere;
};