#include "Pickups/MagnetPickup.h"
#include "Components/PickupRadiusComponent.h"
#include "Kismet/GameplayStatics.h"

void AMagnetPickup::OnPickedUp_Implementation(AActor* PickedUpBy)
{
	Super::OnPickedUp_Implementation(PickedUpBy);

	if (!PickedUpBy)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Magnet] PickedUpBy is null"));
		return;
	}

	UPickupRadiusComponent* PickupRadius = PickedUpBy->FindComponentByClass<UPickupRadiusComponent>();
	if (!PickupRadius)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Magnet] No PickupRadiusComponent found on %s"), *PickedUpBy->GetName());
		return;
	}

	TArray<AActor*> AllPickups;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABasePickup::StaticClass(), AllPickups);

	UE_LOG(LogTemp, Warning, TEXT("[Magnet] Found %d pickups in level"), AllPickups.Num());

	for (AActor* Actor : AllPickups)
	{
		ABasePickup* Pickup = Cast<ABasePickup>(Actor);
		if (!Pickup || Pickup == this) continue;

		UE_LOG(LogTemp, Warning, TEXT("[Magnet] ForcePull on %s"), *Pickup->GetName());
		PickupRadius->ForcePull(Pickup);
	}
}