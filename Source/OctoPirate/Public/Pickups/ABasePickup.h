#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABasePickup.generated.h"

UCLASS()
class OCTOPIRATE_API ABasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	ABasePickup();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void PullToward(const FVector& TargetLocation, float InPullSpeed, AActor* InPlayerRef);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
	void OnPickedUp(AActor* PickedUpBy);
	virtual void OnPickedUp_Implementation(AActor* PickedUpBy);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float PullSpeed = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float PickupTriggerDistance = 50.f;
	
private:
	bool bBeingPulled = false;
	bool bPickedUp = false;

	FVector PullTarget;
	
	UPROPERTY()
	AActor* PlayerRef;
};
