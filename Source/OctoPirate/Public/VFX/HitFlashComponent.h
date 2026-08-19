// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitFlashComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMeshComponent;

/**
 * Flashes every mesh on the owning actor for a split second when it takes a hit.
 *
 * Works by assigning M_HitFlash_Overlay as the meshes' overlay material for the
 * duration of the flash and clearing it again afterwards, so no existing
 * character material needs to know anything about this and there is no cost at
 * all while nothing is being hit.
 *
 * ABaseCharacter creates one of these and fires it from TakeDamage, which covers
 * the player and every enemy. Actors outside that hierarchy (ABossMonkey) can
 * add the component in their Blueprint and call Flash() themselves.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class OCTOPIRATE_API UHitFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitFlashComponent();

	/** Fire the flash. Safe to spam -- a new hit hard-restarts it rather than blending. */
	UFUNCTION(BlueprintCallable, Category = "Combat|VFX")
	void Flash();

	/** Cut the flash immediately. Called on death so corpses don't stay lit. */
	UFUNCTION(BlueprintCallable, Category = "Combat|VFX")
	void StopFlash();

	/** Total flash length in seconds. 0.08 - 0.15 reads well at 60fps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX",
		meta = (ClampMin = "0.0"))
	float Duration = 0.12f;

	/**
	 * Fraction of Duration held at full intensity before the falloff starts.
	 * A pure linear fade with no hold feels mushy.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX",
		meta = (ClampMin = "0.0", ClampMax = "0.9"))
	float HoldFraction = 0.4f;

	/**
	 * Overrides HitFlashColor on the overlay material. Values above 1 are what
	 * push the flash into bloom -- keep the brightest channel well past 1.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
	FLinearColor FlashColor = FLinearColor(5.f, 0.15f, 0.1f, 1.f);

	/** Defaults to MI_HitFlash_Overlay; override per character if you want. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
	TObjectPtr<UMaterialInterface> FlashMaterial;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetAmount(float Amount);
	void ApplyOverlay(UMaterialInterface* Material);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FlashMID;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMeshComponent>> FlashMeshes;

	float TimeRemaining = 0.f;
};
