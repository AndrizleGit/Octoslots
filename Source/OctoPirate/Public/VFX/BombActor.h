// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BombActor.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * A bomb dropped by the Bomb joker. After FuseDelay it explodes once (shared
 * UExplosionStatics::Explode routine), spawns its Niagara VFX, and destroys
 * itself. Subclass as BP_Bomb to add a visual mesh and tune the values.
 */
UCLASS()
class OCTOPIRATE_API ABombActor : public AActor
{
	GENERATED_BODY()

public:
	ABombActor();

	// Damage dealt to each enemy inside Radius when the bomb explodes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	float Damage = 50.f;

	// Radius of the explosion's area-of-effect damage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	float Radius = 300.f;

	// Seconds between the bomb being placed and exploding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	float FuseDelay = 2.f;

	// Niagara system played at the bomb's location on explosion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	// Sound played at the bomb's location on explosion. Leave empty for silence.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	TObjectPtr<USoundBase> ExplosionSound;

	// Volume multiplier applied to ExplosionSound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb", meta = (ClampMin = "0.0"))
	float ExplosionSoundVolume = 1.f;

	// Detonate now: deal AOE damage, play VFX, destroy self. Called by the fuse
	// timer but also exposed so the bomb can be triggered early from Blueprint.
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void Explode();

protected:
	virtual void BeginPlay() override;

	// Empty scene root so BP_Bomb can attach a mesh / idle FX.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USceneComponent> Root;

private:
	FTimerHandle FuseTimer;
	bool bHasExploded = false;
};
