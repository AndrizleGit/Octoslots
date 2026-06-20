// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Upgrades/InRunUpgradeManagerComponent.h"
#include "Upgrades/UpgradeManagerComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/PickupRadiusComponent.h"
#include "OctopusCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS()
class OCTOPIRATE_API AOctopusCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AOctopusCharacter();
	// -- Slot Machine Events -- 
	// - Buffs -
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyAttackSpeedBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyAttackDamageBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyMovementSpeedBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyLifeStealBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyPoisonBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplySevenBuff();

	// Fired when all three reels match. Switch on Symbol to give each three-of-a-kind
	// its own unique payoff. (For SEVEN you can simply call ApplySevenBuff from here.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyThreeOfAKindBuff(ESlotSymbol Symbol);

	// - Debuffs -
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void RemoveBuffs();

	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyDebuff(); 
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ClearDebuff(); 
	
	// - Upgrade Manager -
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Upgrades")
	TObjectPtr<UUpgradeManagerComponent> UpgradeManager;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	UUpgradeManagerComponent* GetUpgradeManager() const { return UpgradeManager; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Upgrades")
	TObjectPtr<UInRunUpgradeManagerComponent> InRunUpgradeManager;
	
	// - Pickup Component -
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UPickupRadiusComponent> PickupRadius;
	    
protected:
	virtual void BeginPlay() override;
	virtual void PerformAttack_Implementation() override;

public:	
	// --- Tentacle Attack ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Tentacle")
	TObjectPtr<USkeletalMeshComponent> TentacleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Tentacle")
	TObjectPtr<UAnimMontage> TentacleAttackMontage;

	// The default scale of the tentacle mesh (tweak this in the editor to get the right base size)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tentacle")
	float BaseTentacleScale = 2.f;

	// Base range used to calculate tentacle scale (set automatically from ConeMaxDistance at BeginPlay)
	float BaseConeMaxDistance = 0.f;

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;
	
	// --- Movement ---
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMoveDestination(const FVector& Destination);
	
	virtual void OnDeath_Implementation() override;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnDeathDelegate OnPlayerDied;
private:
	AActor* GetClosestEnemy() const;

	UFUNCTION()
	void OnTentacleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
