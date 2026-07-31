// Fill out your copyright notice in the Description page of Project Settings.

#include "UObject/ConstructorHelpers.h"
#include "BrokenCannon.h"



// Sets default values
ABrokenCannon::ABrokenCannon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Root Scene
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;
	// Setting Up Base Cube Mesh
	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrokenCannonMesh"));
	CubeMesh->SetupAttachment(RootComponent);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
	   TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded()) CubeMesh->SetStaticMesh(CubeAsset.Object);
	
	
	
	// Setting Up Box Collision
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxCollision->SetupAttachment(RootComponent);
	BoxCollision->SetBoxExtent(FVector(24.f, 23.5f, 23.f));
}

// Called when the game starts or when spawned
void ABrokenCannon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABrokenCannon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

