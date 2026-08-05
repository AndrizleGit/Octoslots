// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blockade.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class OCTOPIRATE_API ABlockade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;
	// Meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes");
	UStaticMeshComponent* CubeMesh;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSubclassOf<UStaticMeshComponent> Box;
	
	// VFX & SFX 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosions")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
	
	// Sound played at the bomb's location on explosion. Leave empty for silence.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosions")
	TObjectPtr<USoundBase> ExplosionSound;

	// Volume multiplier applied to ExplosionSound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosions", meta = (ClampMin = "0.0"))
	float ExplosionSoundVolume = 1.f;
	
	UFUNCTION(BlueprintCallable, Category = "Explosions")
	void Explode();
	
	//Distance from the centre of the Actor, Explosion location = Centre - Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosions")
	FVector ExplosionOffset = FVector(0,0,250.f);

};
