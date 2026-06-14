#include "Pickups/ABasePickup.h"

ABasePickup::ABasePickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABasePickup::BeginPlay()
{
	Super::BeginPlay();

	// Disable all collision on the pickup — movement and pickup are handled in code
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		Comp->SetSimulatePhysics(false);
		Comp->SetEnableGravity(false);
		Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
		Comp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}
}

void ABasePickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bBeingPulled || bPickedUp) return;

	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (PullTarget - CurrentLocation).GetSafeNormal();
	const float Distance = FVector::Dist(CurrentLocation, PullTarget);

	SetActorLocation(CurrentLocation + Direction * PullSpeed * DeltaTime);

	if (Distance <= PickupTriggerDistance)
	{
		bPickedUp = true;
		OnPickedUp(PlayerRef);
		Destroy();
	}
}

void ABasePickup::PullToward(const FVector& TargetLocation, float InPullSpeed, AActor* InPlayerRef)
{
	bBeingPulled = true;
	PullTarget = TargetLocation;
	PullSpeed = InPullSpeed;
	PlayerRef = InPlayerRef;
}

void ABasePickup::OnPickedUp_Implementation(AActor* PickedUpBy)
{
	//Destruction handled by Tick function
}
