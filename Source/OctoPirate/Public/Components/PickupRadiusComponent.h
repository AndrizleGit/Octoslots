#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PickupRadiusComponent.generated.h"

class USphereComponent;
class ABasePickup;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API UPickupRadiusComponent : public UActorComponent
{
	GENERATED_BODY()

public: 
	UPickupRadiusComponent();

protected:
	virtual void BeginPlay() override;

public: 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void UpdateRadius(float NewRadius);
    
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void ForcePull(ABasePickup* Pickup);
private:
	UPROPERTY()
	TObjectPtr<USphereComponent> SphereCollision;
    
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex);
    
	UPROPERTY()
	TArray<ABasePickup*> ActivePickups;
	
	UPROPERTY()
	TArray<ABasePickup*> ForcedPickups;
};