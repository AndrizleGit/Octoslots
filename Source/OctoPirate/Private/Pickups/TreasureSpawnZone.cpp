// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/TreasureSpawnZone.h"
#include "Components/BoxComponent.h"
// Sets default values
ATreasureSpawnZone::ATreasureSpawnZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ZoneBounds = CreateDefaultSubobject<UBoxComponent>("ZoneBounds");
	RootComponent = ZoneBounds;
	ZoneBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneBounds->SetBoxExtent(FVector(1000.f, 1000.f, 500.f));
}

// Called when the game starts or when spawned
void ATreasureSpawnZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATreasureSpawnZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ATreasureSpawnZone::GetRandomPointInZone() const
{
	const FVector Extent = ZoneBounds->GetScaledBoxExtent();
	const FVector LocalRandom = FVector(
		FMath::RandRange(-Extent.X, Extent.X),
		FMath::RandRange(-Extent.Y, Extent.Y),
		0.f
	);
	return ZoneBounds->GetComponentTransform().TransformPosition(LocalRandom);
}

