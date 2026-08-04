// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"

#include "BrokenCannon.generated.h"

class UNiagaraSystem;
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
	bool bAssembled = false;
	// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables|Cannon")
	int CannonFragments = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables|Cannon")
	int MaxFragments = 3;
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> CannonVFX;

};
