#include "Enemy/EnemySpawner.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Enemy/DifficultyManager.h"
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
	
	if (SpawnPool.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnCycle, SpawnCycleInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy Spawner: SpawnPool is empty!"))
	}
}

void AEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
	if (bEnabled) GetWorldTimerManager().UnPauseTimer(SpawnTimerHandle);
	else GetWorldTimerManager().PauseTimer(SpawnTimerHandle);
}

void AEnemySpawner::SpawnCycle()
{
	ActiveEnemies.RemoveAll([](ABaseEnemyCharacter* Enemy)
	{
		return !IsValid(Enemy);
	});
	
	if (ActiveEnemies.Num() >= MaxEnemies) return;
	
	ADifficultyManager* DifficultyManager = ADifficultyManager::Get(GetWorld());
	const float Coefficient = DifficultyManager ? DifficultyManager->GetDifficultyCoefficient() : 0.f;
	float Budget = DifficultyManager ? DifficultyManager->GetCurrentSpawnBudget() : 4.f;
	
	UE_LOG(LogTemp, Error, TEXT("SpawnCycle — DifficultyManager: %s, Coefficient: %.2f, Budget: %.2f"),
		DifficultyManager ? TEXT("FOUND") : TEXT("NULL"), Coefficient, Budget);
	
	while (Budget > 0.0f && ActiveEnemies.Num() < MaxEnemies)
	{
		UEnemySpawnData* Choice = PickWeightedEnemy(Coefficient);
		
		UE_LOG(LogTemp, Error, TEXT("PickWeightedEnemy returned: %s"), Choice ? TEXT("VALID") : TEXT("NULL"));

		if (!Choice || !Choice->EnemyClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Breaking — Choice null or EnemyClass null"));
			break;
		}
		
		Budget -= Choice->SpawnCost;
		if (Budget < 0.f)
		{
			UE_LOG(LogTemp, Error, TEXT("Breaking — Budget went negative: %.2f"), Budget);
			break;
		}
		
		const FVector SpawnLocation = GetSpawnLocationOutsideViewport();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		ABaseEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(Choice->EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		
		UE_LOG(LogTemp, Error, TEXT("SpawnActor result: %s at %s"), NewEnemy ? TEXT("SUCCESS") : TEXT("FAILED"), *SpawnLocation.ToString());

		if (NewEnemy)
		{
			ActiveEnemies.Add(NewEnemy);
			
			if (DifficultyManager)
			{
				NewEnemy->ApplyDifficultyScaling(DifficultyManager->GetHealthMultiplier(), DifficultyManager->GetDamageMultiplier());
			}
		}
	}
}

UEnemySpawnData* AEnemySpawner::PickWeightedEnemy(float DifficultyCoefficient) const
{
	TArray<UEnemySpawnData*> EligiblePool;
	float TotalWeight = 0.f;
	
	for (UEnemySpawnData* Entry : SpawnPool)
	{
		if (Entry && DifficultyCoefficient > Entry->MinDifficultyToSpawn)
		{
			EligiblePool.Add(Entry);
			TotalWeight += (1.f / FMath::Max(Entry->SpawnCost, 0.01f));
		}
	}
	
	if (EligiblePool.Num() == 0) return nullptr;
	
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (UEnemySpawnData* Entry : EligiblePool)
	{
		const float Weight = 1.f / FMath::Max(Entry->SpawnCost, 0.01f);
		if (Roll <= Weight) return Entry;
		Roll -= Weight;
	}
	
	return EligiblePool.Last();
}

FVector AEnemySpawner::GetSpawnLocationOutsideViewport() const
{
	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	const FVector PlayerLocation = Player ? Player->GetActorLocation() : FVector::ZeroVector;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return FVector::ZeroVector;

	for (int32 Attempt = 0; Attempt < 20; Attempt++)
	{
		const float RandomAngle = FMath::RandRange(0.f, 360.f);
		FVector CandidateLocation = PlayerLocation + FVector(
			FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance,
			FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * SpawnDistance,
			0.f
		);
		
		CandidateLocation.Z = PlayerLocation.Z + 5000.f;

		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(
			CandidateLocation, NavLocation, FVector(500.f, 500.f, 5000.f)))
		{
			continue;
		}

		const float HorizontalDist = FVector::Dist2D(NavLocation.Location, PlayerLocation);
		if (HorizontalDist < SpawnDistance * 0.5f)
		{
			continue;
		}

		return NavLocation.Location;
	}

	UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: Failed to find valid spawn location"));
	return FVector::ZeroVector;
}