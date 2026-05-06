// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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
	ProjectileComponent->MaxSpeed = 1100;
	ProjectileComponent->ProjectileGravityScale = 0;

}

void APlayerProjectile::BeginPlay()
{
	Super::BeginPlay();

	// inherit the shooter's current damage so buffs apply automatically
	if (const AOctoPirateCharacter* Player = Cast<AOctoPirateCharacter>(GetInstigator()))
	{
		Damage = Player->CurrentDamage;
	}
}

void APlayerProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	UGameplayStatics::ApplyDamage(Other, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
	Destroy();
}

// Called every frame
void APlayerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

