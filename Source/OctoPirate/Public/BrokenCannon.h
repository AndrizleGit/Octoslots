// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"


#include "BrokenCannon.generated.h"
class ABlockade;
class UNiagaraSystem;
class USoundBase;
UCLASS()

class OCTOPIRATE_API ABrokenCannon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABrokenCannon();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;
	// Meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes");
	UStaticMeshComponent* CubeMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* BoxCollision;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//bools
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables|Cannon")
	bool bIsActive = true;
	// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables|Cannon")
	int32 CannonBall = 0;
	// List Of blockade linked to this cannon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables|Cannon")
	TArray<TObjectPtr<ABlockade>> BlockadeList;
	
	
	// VFX & SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> CannonVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TObjectPtr<USoundBase> CannonSFX;
	
	// Volume multiplier applied to ExplosionSound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (ClampMin = "0.0"))
	float ExplosionSoundVolume = 1.f;


	
	
};
