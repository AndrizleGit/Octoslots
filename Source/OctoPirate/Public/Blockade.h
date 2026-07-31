// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blockade.generated.h"

class UNiagaraSystem;
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
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
};
