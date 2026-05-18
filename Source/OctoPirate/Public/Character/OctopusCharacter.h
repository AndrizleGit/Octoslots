// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character\AttributeSets\BasicAttributeSet.h"

#include "OctopusCharacter.generated.h"


UCLASS()
class OCTOPIRATE_API AOctopusCharacter : public ACharacter, public IAbilitySystemInterface 
{
	GENERATED_BODY()

public:
	AOctopusCharacter();
	
	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributes;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;
	
	// --- Combat ---
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
	
	
	// -- Ability System --
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override; 
	
private:
	void PerformAttack();
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage);
	float GetAttackSpeed() const;
	
	FTimerHandle AttackTimerHandle;
};
