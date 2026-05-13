// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OctoPirateCharacter.h"
#include "PlayerProjectile.generated.h"

UCLASS()
class OCTOPIRATE_API APlayerProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float Damage = 1.f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ProjectileMesh;
	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileComponent;
};
