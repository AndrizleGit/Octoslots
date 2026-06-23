// Fill out your copyright notice in the Description page of Project Settings.

#include "VFX/ExplosionStatics.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UExplosionStatics::Explode(
	const UObject* WorldContextObject,
	FVector Location,
	float Radius,
	float Damage,
	UNiagaraSystem* NiagaraSystem,
	AController* InstigatorController,
	AActor* DamageCauser,
	const TArray<AActor*>& IgnoreActors)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World) return;

	// -- VFX (plays regardless of whether anything is in range) --
	if (NiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, NiagaraSystem, Location, FRotator::ZeroRotator, FVector(1.f), true, true);
	}

	if (Radius <= 0.f || Damage <= 0.f) return;

	// -- AOE damage: enemies only (ActorClassFilter restricts the overlap so the
	//    player is never caught in the blast) --
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> Overlapped;
	UKismetSystemLibrary::SphereOverlapActors(
		World, Location, Radius, ObjectTypes,
		ABaseEnemyCharacter::StaticClass(), IgnoreActors, Overlapped);

	for (AActor* Actor : Overlapped)
	{
		if (!Actor) continue;
		UGameplayStatics::ApplyDamage(Actor, Damage, InstigatorController, DamageCauser, UDamageType::StaticClass());
	}
}
