#include "Enemy/EnemySpawner.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (EnemyClass)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy Spawner: No EnemyClass set!"))
	}
}

void AEnemySpawner::SetSpawnInterval(float NewInterval)
{
	SpawnInterval = NewInterval;
	
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy,SpawnInterval,true);
}

void AEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
	if (bEnabled) GetWorldTimerManager().UnPauseTimer(SpawnTimerHandle);
	else GetWorldTimerManager().PauseTimer(SpawnTimerHandle);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass) return;
	
	ActiveEnemies.RemoveAll([](ABaseEnemyCharacter* Enemy)
	{
		return !IsValid(Enemy);
	});
	
	if (ActiveEnemies.Num() >= MaxEnemies) return;
	
	const FVector SpawnLocation = GetSpawnLocationOutsideViewport();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ABaseEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(EnemyClass,SpawnLocation,FRotator::ZeroRotator,SpawnParams);
	
	if (NewEnemy)
	{
		ActiveEnemies.Add(NewEnemy);
	}
}

FVector AEnemySpawner::GetSpawnLocationOutsideViewport() const
{
	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	const FVector PlayerLocation = Player ? Player->GetActorLocation() : FVector::ZeroVector;
	
	const float RandomAngle = FMath::RandRange(0.f,360.f);
	FVector SpawnLocation = PlayerLocation + FVector(
		FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance,
		FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance, 0.f
		);
	
	SpawnLocation.Z = PlayerLocation.Z;
	
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(200.f,200.f,200.f)))
		{
			SpawnLocation = NavLocation.Location;
		}
	}
	
	return SpawnLocation;
}