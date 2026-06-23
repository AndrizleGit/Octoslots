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
#include "SlotMachineTypes.h"
#include "OctopusCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

// -- Tags --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PoisonImmune)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PlayerPoison)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PoisonWeaponBuff)
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
	
	// -- Extra abilities -- 
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> PoisonEffectClass;
	
	UPROPERTY()
	FGameplayEffectSpecHandle CachedPoisonSpecHandle;
	
	// - Joker Properties -
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|Scrooge")
	float ScroogeDamagePerCoin = 0.1f;

	// - Joker: Bomb (EffectID "BombDrop") -
	// While this joker is active a BombClass actor is dropped behind the player
	// every BombSpawnInterval seconds. Per-bomb Damage/Radius/FuseDelay/VFX live
	// on the BombClass (BP_Bomb) itself.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|Bomb")
	TSubclassOf<class ABombActor> BombClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|Bomb")
	float BombSpawnInterval = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|Bomb")
	float BombSpawnDistanceBehind = 150.f;

	// - Joker: Explode on Death (EffectID "DeathExplosion") -
	// Independent values from the Bomb joker; read by ABaseEnemyCharacter::OnDeath
	// when this joker is active.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|DeathExplosion")
	float DeathExplosionDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|DeathExplosion")
	float DeathExplosionRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|DeathExplosion")
	TObjectPtr<class UNiagaraSystem> DeathExplosionVFX;
	
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
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage) override;
	static int32 GetStacksByTag(UAbilitySystemComponent* ASC, FGameplayTag EffectTag) ;

	// React to the bomb joker being granted/removed.
	virtual void OnJokerEffectAdded(FName EffectID, float Value) override;
	virtual void OnJokerEffectRemoved(FName EffectID) override;

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

	// Starts the bomb-drop timer when the "BombDrop" joker is active, stops it otherwise.
	void RefreshBombTimer();

	UFUNCTION()
	void SpawnBombBehind();

	FTimerHandle BombSpawnTimer;

	UFUNCTION()
	void OnTentacleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
