#pragma once

#include "CoreMinimal.h"
#include "Pickups/ABasePickup.h"
#include "MagnetPickup.generated.h"

UCLASS()
class OCTOPIRATE_API AMagnetPickup : public ABasePickup
{
	GENERATED_BODY()

public:
	virtual void OnPickedUp_Implementation(AActor* PickedUpBy) override;
};