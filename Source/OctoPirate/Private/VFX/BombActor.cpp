// Fill out your copyright notice in the Description page of Project Settings.

#include "VFX/BombActor.h"
#include "VFX/ExplosionStatics.h"
#include "Engine/World.h"

ABombActor::ABombActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	if (FuseDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(FuseTimer, this, &ABombActor::Explode, FuseDelay, false);
	}
	else
	{
		Explode();
	}
}

void ABombActor::Explode()
{
	if (bHasExploded) return;
	bHasExploded = true;

	// Credit the player (our instigator) for any kills so XP and drops are awarded.
	UExplosionStatics::Explode(
		this,
		GetActorLocation(),
		Radius,
		Damage,
		ExplosionVFX,
		GetInstigatorController(),
		this,
		{ this });

	Destroy();
}
