// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/OctoPirateEnemySpawner.h"

#include "Enemy/OctoPirateEnemyCharacter.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

AOctoPirateEnemySpawner::AOctoPirateEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOctoPirateEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemySpawner] EnemyClass is not set — no enemies will spawn!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemySpawner] Started. Interval=%.1fs MaxEnemies=%d"), SpawnInterval, MaxEnemies);
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AOctoPirateEnemySpawner::SpawnEnemy, SpawnInterval, true);
}

void AOctoPirateEnemySpawner::SpawnEnemy()
{
	UE_LOG(LogTemp, Log, TEXT("[EnemySpawner] Trying to spawn. Live enemies: %d / %d"), GetLiveEnemyCount(), MaxEnemies);

	if (GetLiveEnemyCount() >= MaxEnemies)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemySpawner] At max enemy count, skipping."));
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] No player pawn found."));
		return;
	}

	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemySpawner] No NavigationSystem found — is a NavMesh in the level?"));
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();

	FNavLocation NavLocation;
	bool bFoundSpawnPoint = false;
	for (int32 Attempt = 0; Attempt < 5; ++Attempt)
	{
		const FVector RandomDir = FMath::VRand().GetSafeNormal2D();
		const float Distance = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
		const FVector SearchOrigin = PlayerLocation + RandomDir * Distance;

		if (!NavSys->GetRandomPointInNavigableRadius(SearchOrigin, MaxSpawnDistance * 0.5f, NavLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] Attempt %d: No navmesh point found near %s"), Attempt + 1, *SearchOrigin.ToString());
			continue;
		}

		const float DistToPlayer = FVector::Dist(NavLocation.Location, PlayerLocation);
		if (DistToPlayer < MinSpawnDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] Attempt %d: Navmesh point too close to player (%.0f units)"), Attempt + 1, DistToPlayer);
			continue;
		}

		bFoundSpawnPoint = true;
		UE_LOG(LogTemp, Log, TEXT("[EnemySpawner] Attempt %d: Found spawn point at %s (%.0f units from player)"), Attempt + 1, *NavLocation.Location.ToString(), DistToPlayer);
		break;
	}

	if (!bFoundSpawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] All 5 attempts failed — could not find a valid spawn location."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// raise spawn point above the navmesh surface so the capsule doesn't clip the floor
	const FVector SpawnLocation = NavLocation.Location + FVector(0.f, 0.f, 90.f);

	AOctoPirateEnemyCharacter* Enemy = GetWorld()->SpawnActor<AOctoPirateEnemyCharacter>(
		EnemyClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Enemy)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemySpawner] Successfully spawned enemy at %s"), *NavLocation.Location.ToString());
		SpawnedEnemies.Add(Enemy);
		Enemy->OnEnemyDied.AddLambda([this, Enemy]()
		{
			SpawnedEnemies.Remove(Enemy);
		});
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] SpawnActor returned null — likely a collision block at spawn point."));
	}
}

int32 AOctoPirateEnemySpawner::GetLiveEnemyCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AOctoPirateEnemyCharacter>& Enemy : SpawnedEnemies)
	{
		if (Enemy.IsValid()) ++Count;
	}
	return Count;
}
