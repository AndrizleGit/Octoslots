// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h" 

// Sets default values
APlayerProjectile::APlayerProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Component"));
	// Projectile Params
	ProjectileComponent->InitialSpeed = 1000;
	ProjectileComponent->MaxSpeed = 1000;

}

// Called when the game starts or when spawned
void APlayerProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

