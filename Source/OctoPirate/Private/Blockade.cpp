// Fill out your copyright notice in the Description page of Project Settings.

#include "VFX/ExplosionStatics.h"

#include "Blockade.h"

// Sets default values
ABlockade::ABlockade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Root Scene
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;
	// Setting Up Base Cube Mesh
	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Blockade"));
	CubeMesh->SetupAttachment(RootComponent);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
	   TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded()) CubeMesh->SetStaticMesh(CubeAsset.Object);
	
}

// Called when the game starts or when spawned
void ABlockade::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABlockade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABlockade::Explode()
{
	
	FVector ExplosionLocation = GetActorLocation() - ExplosionOffset;
	UExplosionStatics::Explode(
		this,
		ExplosionLocation,
		0,
		0,
		ExplosionVFX,
		ExplosionSound,
		GetInstigatorController(),
		this,
		{ this },
		ExplosionSoundVolume);
}


