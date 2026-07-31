// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExplosionStatics.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * Shared explosion routine reused by the Bomb joker (ABombActor) and the
 * explode-on-death joker (ABaseEnemyCharacter::OnDeath). Damages enemies only.
 */
UCLASS()
class OCTOPIRATE_API UExplosionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Spawns NiagaraSystem and plays ExplosionSound at Location (both optional),
	// then deals Damage to every enemy pawn within Radius.
	// InstigatorController/DamageCauser are credited for the kill (XP, drops);
	// pass the player's controller so explosion kills count.
	UFUNCTION(BlueprintCallable, Category = "Explosion", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreActors"))
	static void Explode(
		const UObject* WorldContextObject,
		FVector Location,
		float Radius,
		float Damage,
		UNiagaraSystem* NiagaraSystem,
		USoundBase* ExplosionSound,
		AController* InstigatorController,
		AActor* DamageCauser,
		const TArray<AActor*>& IgnoreActors,
		float SoundVolumeMultiplier = 1.f);
};
