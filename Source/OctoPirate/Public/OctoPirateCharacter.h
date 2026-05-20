// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OctoPirateCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class AOctoPirateCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void AddMovementSpeedBonus(float Bonus);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void RemoveMovementSpeedBonus();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyAttackSpeedMultiplier(float Multiplier);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void RemoveAttackSpeedMultiplier();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Slot Machine|Player")
	void AddAttackDamageBonus(float Bonus);

	UFUNCTION(BlueprintNativeEvent, Category = "Slot Machine|Player")
	void RemoveAttackDamageBonus();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentDamage = 1.f;

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

public:

	/** Constructor */
	AOctoPirateCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

	
	
};

