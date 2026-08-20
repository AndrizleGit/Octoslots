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

enum class EPoisonProjectileEffect : uint8;

// -- Tags --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PoisonImmune)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PlayerPoison)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PoisonWeaponBuff)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_PoisonTrailBuff)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Debuffs_PoisonTrailDebuff)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeflectCooldownChanged, float, NormalizedValue);

UCLASS()
class OCTOPIRATE_API AOctopusCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AOctopusCharacter();
	
	// --- Audio ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DeflectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DeathSound;
	
	// -- Slot Machine Events -- 
	// - Buffs -
	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyAttackDamageBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplySpeedBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyLifeStealBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplyPoisonBuff(int32 StackCount);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot Machine|Player")
	void ApplySevenBuff();
	// -- Treasuremap Events --
	UFUNCTION(BlueprintImplementableEvent, Category = "Treasure Map")
	void OnTreasureSpawn();
	
	// -- Check for Tag Changes -- 
	UFUNCTION(BlueprintImplementableEvent)
	void OnTagChanged(FGameplayTag Tag, int32 NewCount);

	
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
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> PoisonPathEffectClass;
	UPROPERTY()
	FGameplayEffectSpecHandle CachedPoisonPathSpecHandle;
	
	// - Poison Weapon Projectile -
	// While Buffs.PoisonWeapon is active, every attack swing fires one of these forward.
	// Travel distance, speed, slowdown and the Niagara system live on the projectile
	// Blueprint itself (BP_PoisonProjectile).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Projectile")
	TSubclassOf<class APoisonProjectile> PoisonProjectileClass;

	// How far in front of the character the ball spawns.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Projectile")
	float PoisonProjectileSpawnOffset = 60.f;

	// Height offset from the character origin, so the ball can fly at ground level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Projectile")
	float PoisonProjectileSpawnHeight = 0.f;

	// The projectile is now the thing that poisons. Set this to true to *also* keep the
	// old behaviour where the melee cone poisons everything it hits.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Projectile")
	bool bMeleeSwingAlsoAppliesPoison = false;

	// Fires one poison ball forward. Called automatically on every attack swing while
	// the poison buff is active; also exposed for anim notifies.
	UFUNCTION(BlueprintCallable, Category = "Effects|Poison Projectile")
	void SpawnPoisonProjectile();

	// Applies the poison weapon effect (once per buff stack) to Target.
	// Returns true when the poison was actually applied. Used by APoisonProjectile.
	UFUNCTION(BlueprintCallable, Category = "Effects|Poison Projectile")
	bool ApplyPoisonToActor(AActor* Target);
	
	// - Three Of A Kind Poison Volley -
	// With Buffs.PoisonTrail active the swing fires a fan of balls instead of a single
	// one: one straight ahead and the rest split evenly to either side.
	// Falls back to PoisonProjectileClass when left empty.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Volley")
	TSubclassOf<class APoisonProjectile> PoisonVolleyProjectileClass;

	// Number of balls in the fan. 3 = one straight plus one to each side.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Volley", meta = (ClampMin = "1"))
	int32 PoisonVolleyCount = 3;

	// Angle between neighbouring balls, in degrees. With a count of 3 and 30 degrees
	// the fan comes out at -30 / 0 / +30.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Volley", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PoisonVolleySpreadDegrees = 30.f;

	// Travel distance for volley balls, overriding the value on their Blueprint.
	// Set to 0 to use whatever the Blueprint itself says.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Poison Volley", meta = (ClampMin = "0.0"))
	float PoisonVolleyTravelDistance = 1600.f;

	// The stronger poison the volley applies. Leave empty to reuse PoisonPathEffectClass,
	// the effect the three-of-a-kind already applied on hit.
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Poison Volley")
	TSubclassOf<UGameplayEffect> ThreeOfAKindPoisonEffectClass;
	UPROPERTY()
	FGameplayEffectSpecHandle CachedThreeOfAKindSpecHandle;

	// Fires the whole fan. Called automatically on every attack swing while the
	// three-of-a-kind poison buff is active; also exposed for anim notifies.
	UFUNCTION(BlueprintCallable, Category = "Effects|Poison Volley")
	void SpawnPoisonVolley();

	// Applies the three-of-a-kind poison to Target. Used by APoisonProjectile.
	UFUNCTION(BlueprintCallable, Category = "Effects|Poison Volley")
	bool ApplyThreeOfAKindPoisonToActor(AActor* Target);
	
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

	// Sound played at the dying enemy's location. Leave empty for silence.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|DeathExplosion")
	TObjectPtr<USoundBase> DeathExplosionSound;

	// Volume multiplier applied to DeathExplosionSound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jokers|DeathExplosion", meta = (ClampMin = "0.0"))
	float DeathExplosionSoundVolume = 1.f;
	
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
	
	// --- Deflect System ---

	// activates/deactivates the deflect window — can be set from slot machine buff or input
	UFUNCTION(BlueprintCallable, Category = "Combat|Deflect")
	void SetDeflectActive(bool bActive);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Deflect")
	bool IsDeflectActive() const { return bDeflectActive; }

	// how long the deflect window stays open on left click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Deflect")
	float DeflectDuration = 0.5f;

	// placeholder — replace with actual animation call when ready
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Deflect")
	void OnDeflectStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Deflect")
	void OnDeflectEnded();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Deflect")
	float DeflectCooldown = 3.f;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Deflect")
	bool CanDeflect() const { return bCanDeflect; }
	    
	UPROPERTY(BlueprintAssignable, Category = "Combat|Deflect")
	FOnDeflectCooldownChanged OnDeflectCooldownChanged;
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Deflect")
	void TriggerDeflect();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float AttackTriggerBuffer = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UNiagaraSystem> DeathVFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Attack")
	TObjectPtr<class UDecalComponent> AttackRangeDecal;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PerformAttack_Implementation() override;
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage) override;
	static int32 GetStacksByTag(UAbilitySystemComponent* ASC, FGameplayTag EffectTag) ;

	// Shared by the melee cone and the poison projectiles.
	bool ApplyPoisonWeaponToASC(UAbilitySystemComponent* TargetASC);
	bool ApplyThreeOfAKindPoisonToASC(UAbilitySystemComponent* TargetASC);

	// Spawns one ball, yawed off the character's facing by YawOffsetDegrees.
	void SpawnPoisonProjectileAtAngle(TSubclassOf<class APoisonProjectile> ProjectileClass,
		float YawOffsetDegrees, EPoisonProjectileEffect EffectMode, float TravelDistanceOverride);

	// React to the bomb joker being granted/removed.
	virtual void OnJokerEffectAdded(FName EffectID, float Value) override;
	virtual void OnJokerEffectRemoved(FName EffectID) override;
public:	
	// --- Tentacle Attack ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Tentacle")
	TObjectPtr<USkeletalMeshComponent> TentacleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Tentacle")
	TObjectPtr<UAnimMontage> TentacleAttackMontage;

	// Yaw correction (degrees) applied on top of the aim rotation, to align the
	// tentacle mesh's authored forward axis with the attack direction. Tune live
	// in BP_OctopusCharacter; try ±90 if the tentacle points sideways.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tentacle")
	float TentacleYawOffset = -45.f;

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
	
	// -- Animation -- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bHelicopterMode = false;
	
private:
	AActor* GetClosestEnemy() const;

	// Starts the bomb-drop timer when the "BombDrop" joker is active, stops it otherwise.
	void RefreshBombTimer();

public:
	// Console test cheat: grant a joker effect immediately, e.g. `GrantJoker BombDrop`
	// or `GrantJoker DeathExplosion` in the PIE console (~). Lets you test without
	// grinding to level 10. Remove before shipping.
	UFUNCTION(Exec)
	void GrantJoker(FName EffectID, float Value = 0.f);

private:

	UFUNCTION()
	void SpawnBombBehind();

	FTimerHandle BombSpawnTimer;
	bool bDeflectActive = false;
	FTimerHandle DeflectTimer;
	
	UFUNCTION()
	void OnTentacleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void DeactivateDeflect();
	
	bool bCanDeflect = true;
	float CooldownRemaining = 0.f;
};
