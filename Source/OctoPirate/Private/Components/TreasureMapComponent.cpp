// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TreasureMapComponent.h"

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
	
}


// Called every frame
void UTreasureMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTreasureMapComponent::AddCannonFragment()
{
	CannonFragment += 1; 
}

int UTreasureMapComponent::SubmitCannonFragment()
{
	int currentCannonFragment = CannonFragment;
	CannonFragment = 0;
	return currentCannonFragment;
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
			DesiredPoint, NavLocation, FVector(SearchExtent, SearchExtent, SearchExtent));

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

