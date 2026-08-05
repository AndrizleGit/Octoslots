#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "NiagaraSystem.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "BaseEnemyCharacter.generated.h"

UCLASS()
class OCTOPIRATE_API ABaseEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	ABaseEnemyCharacter();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Progression")
	float ExperienceReward = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackRange = 350.f;

	// Montage played on the character mesh each time this enemy attacks in range.
	// Assign per-enemy in the Blueprint (e.g. the monkey's attack montage).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	TObjectPtr<class UAnimMontage> AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	TSubclassOf<AActor> CoinClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	TSubclassOf<AActor> HealthPackClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	float HealthPackDropChance = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	TSubclassOf<AActor> TreasuremapClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	float TreasuremapDropChance = 0.1f;
	
	UFUNCTION(BlueprintCallable, Category = "Enemy|Difficulty")
	void ApplyDifficultyScaling(float HealthMultiplier, float DamageMultiplier);
	
	UFUNCTION(BlueprintCallable, Category = "Enemy|Status")
	void Freeze(float Duration);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|VFX")
	TObjectPtr<UNiagaraSystem> SpawnVFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float BaseMaxHealth = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float BaseAttackDamage = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float WalkSpeedMultiplier = 1.2f;
	
	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void SetMovementLocked(bool bLocked) { bMovementLocked = bLocked; }
	
	virtual void OnDeath_Implementation() override;
	
protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack_Implementation() override;
	
	void ChasePlayer();
	
	UPROPERTY()
	ACharacter* PlayerCharacter;
	
	void UnFreeze();
	
	FTimerHandle FreezeTimerHandle;
	
	bool bIsFrozen = false;
	
	bool bMovementLocked = false;
};
