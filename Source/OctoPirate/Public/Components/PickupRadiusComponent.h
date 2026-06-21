// Fill out your copyright notice in the Description page of Project Settings.

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
	
private:
	UPROPERTY()
	TObjectPtr<USphereComponent> SphereCollision;
	
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY()
	TArray<ABasePickup*> ActivePickups;
};
