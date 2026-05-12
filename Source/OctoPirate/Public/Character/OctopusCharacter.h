// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "OctopusCharacter.generated.h"

UCLASS()
class OCTOPIRATE_API AOctopusCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOctopusCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;
	
	// --- Combat ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float AttackInterval = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Combat|Attack")
	float ConeAngleDegrees = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Combat|Attack")
	float SwordStartDistance = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Combat|Attack")
	float ConeMaxDistance = 350.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Combat|Attack")
	float TentacleDamage = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Combat|Attack")
	float SwordDamage = 45.f;
	
	// --- Movement ---
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMoveDestination(const FVector& Destination);
	
private:
	void PerformAttack();
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage);
	
	FTimerHandle AttackTimerHandle;
};
