// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TreasureMapComponent.h"
#include "Pickups/TreasureSpawnZone.h"
#include "Kismet/GameplayStatics.h"
// Sets default values for this component's properties
UTreasureMapComponent::UTreasureMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTreasureMapComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AOctopusCharacter>(GetOwner());
	FindAllTreasureSpawnZones();
	
}


// Called every frame
void UTreasureMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTreasureMapComponent::FindAllTreasureSpawnZones() 
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATreasureSpawnZone::StaticClass(),FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (ATreasureSpawnZone* TreasureSpawnZone = Cast<ATreasureSpawnZone>(Actor))
		{
			TreasureSpawnZones.Add(TreasureSpawnZone);
		}
	}
}

void UTreasureMapComponent::NextTreasureNumber()
{
	if (!PlayerCharacter) return;
	
	TreasureNumber += 1;
}

void UTreasureMapComponent::AddCannonAmmo()
{
	CannonFragment += 1; 
}

int UTreasureMapComponent::SubmitCannonAmmo()
{
	int currentCannonFragment = CannonFragment;
	
	CannonFragment = 0;
	return currentCannonFragment;
}
// Pick a Zone Close to the player with the same Level as Treasure Level and spawn a Treasure there
void UTreasureMapComponent::SpawnTreasureInZone()
{
	if (!PlayerCharacter || !TreasureClass) return;
	UWorld* World = GetWorld();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FVector SpawnLocation = GetSpawnLocation(GetRandomTreasureSpawnZone());
	World->SpawnActor<AActor>(TreasureClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	PlayerCharacter->OnTreasureSpawn();
}
// Returns TreasureZone that are equal in Level with TreasureLevel and are closest to the Player
ATreasureSpawnZone* UTreasureMapComponent::GetClosestTreasureSpawnZone() const
{
	ATreasureSpawnZone* ClosestZone = nullptr;
	float ClosestDistance = FLT_MAX;
	for (ATreasureSpawnZone* Zone : TreasureSpawnZones)
	{
		if (!Zone) continue;

		const float Dist = FVector::Dist(PlayerCharacter->GetActorLocation(), Zone->GetActorLocation());
		
		if (Dist < ClosestDistance || TreasureNumber == Zone->GetZoneNumber())
		{
			ClosestDistance = Dist;
			ClosestZone = Zone;
		}
	}
	return ClosestZone;
}
// Return a Random TreasureZone of the same level
ATreasureSpawnZone* UTreasureMapComponent::GetRandomTreasureSpawnZone() const
{
	TArray<TObjectPtr<ATreasureSpawnZone>> SortedTreasureSpawnZones;
	
	for (ATreasureSpawnZone* Zone : TreasureSpawnZones)
	{
		if (!Zone) continue;
		if (TreasureNumber == Zone->GetZoneNumber())
		{
			SortedTreasureSpawnZones.Add(Zone);
		}
		
	}
	int32 index = FMath::RandRange(0, SortedTreasureSpawnZones.Num() - 1);
	return SortedTreasureSpawnZones[index];
}
//Pick a random Location that's on the NavMesh inside the TreasureZone
FVector UTreasureMapComponent::GetSpawnLocation(ATreasureSpawnZone* TreasureSpawnZone) const
{
	if (!TreasureSpawnZone) return FVector::ZeroVector;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return FVector::ZeroVector;

	for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
	{
		FVector CandidateLocation = TreasureSpawnZone->GetRandomPointInZone();

		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(
			CandidateLocation, NavLocation, SearchExtent))
		{
			continue;
		}

		return NavLocation.Location;
	}

	UE_LOG(LogTemp, Warning, TEXT("TreasureMap Component: Failed to find spawn location in zone %s"), *TreasureSpawnZone->GetName());
	return FVector::ZeroVector;
}

void UTreasureMapComponent::SpawnTreasure()
{
	if (!PlayerCharacter || !TreasureClass) return;
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	
	if (!NavSys) return;

	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const FVector Direction(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		const FVector DesiredPoint = PlayerLocation + Direction * Distance;

		FNavLocation NavLocation;
		const bool bOnNavMesh = NavSys->ProjectPointToNavigation(
			DesiredPoint, NavLocation, SearchExtent);

		if (!bOnNavMesh)
		{
			continue;
		}

		// Reachability check
		const bool bPathExists = NavSys->TestPathSync(
		FPathFindingQuery(PlayerCharacter, *NavSys->GetDefaultNavDataInstance(), PlayerLocation, NavLocation.Location)
		);

		if (!bPathExists) continue;

		// Border clearance check
		if (!IsPointAwayFromNavMeshBorder(NavSys, NavLocation.Location, BorderClearance))
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		UE_LOG(LogTemp, Warning, TEXT("Treasure Spawned !!"));
		World->SpawnActor<AActor>(TreasureClass, NavLocation.Location, FRotator::ZeroRotator, SpawnParams);
		PlayerCharacter->OnTreasureSpawn();
		return;
	}

	
}



bool UTreasureMapComponent::IsPointAwayFromNavMeshBorder(UNavigationSystemV1* NavSys, const FVector& Point, float ClearanceRadius, int32 NumSamples)
{
    for (int32 i = 0; i < NumSamples; ++i)
    {
        const float Angle = (2.f * PI / NumSamples) * i;
        const FVector Offset(FMath::Cos(Angle) * ClearanceRadius, FMath::Sin(Angle) * ClearanceRadius, 0.f);
        const FVector SamplePoint = Point + Offset;

        FNavLocation SampleNavLocation;

        // Use a small search extent so we don't accidentally "snap back" across an edge
        const bool bValid = NavSys->ProjectPointToNavigation(
            SamplePoint,
            SampleNavLocation,
            FVector(10.f, 10.f, 100.f) // tight horizontal tolerance, generous vertical
        );

        if (!bValid)
        {
            return false; // off the mesh entirely -> too close to an edge/hole
        }

        // If the projected point drifted far horizontally, it means the real nav mesh
        // boundary curves away nearby and got "snapped" from elsewhere
        const float HorizontalDrift = FVector::DistSquared2D(SampleNavLocation.Location, SamplePoint);
        if (HorizontalDrift > FMath::Square(15.f))
        {
            return false;
        }
    }

    return true;
}

