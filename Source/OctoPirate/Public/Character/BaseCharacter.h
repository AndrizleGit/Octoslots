// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "NativeGameplayTags.h"
#include "BaseCharacter.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Combat_Hit)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Immortal)

	
UCLASS()
class OCTOPIRATE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	
	// Callback Functions
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	virtual void OnAttackSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void OnAttackDamageChanged(const FOnAttributeChangeData& Data);
	virtual void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void OnExperienceChanged(const FOnAttributeChangeData& Data);
	virtual void OnLifeStealChanged(const FOnAttributeChangeData& Data);
	virtual void OnPickupRadiusChanged(const FOnAttributeChangeData& Data);
public:	
	virtual void Tick(float DeltaTime) override;
	
	
	
	// -- Ability System Component --
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributes;
	
	// --- Damage Numbers ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|VFX")
	TSubclassOf<class ADamageNumberActor> DamageNumberClass;
	
	
	// --- Combat ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float AttackInterval = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float ConeAngleDegrees = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float ExtraDamageDistance = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float ConeMaxDistance = 350.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float ExtraDamage = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float KnockbackStrength = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float KnockbackDuration = 0.15f;
	// --- Joker ---
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Jokers")
	bool HasJokerEffect(FName EffectID) const { return ActiveJokerEffects.Contains(EffectID); }
	
	UFUNCTION(BlueprintCallable, Category = "Jokers")
	void AddJokerEffect(FName EffectID) { ActiveJokerEffects.AddUnique(EffectID); }
	
	UFUNCTION(BlueprintCallable, Category = "Jokers")
	void RemoveJokerEffect(FName EffectID) { ActiveJokerEffects.Remove(EffectID); }
	
	
	// --- Functions ---
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealthPercent() const;
	
	
	
	// -- Get Attribute functions -- 
	float GetAttackSpeed() const;
	float GetAttackDamage() const;
	float GetHealth() const;
	float GetMaxHealth() const;
	float GetWalkSpeed() const;
	
	
protected:
	
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void PerformAttack();
	virtual void PerformAttack_Implementation();
	
	virtual void ApplyDamageInZone(float MinDist, float MaxDist, float Damage);
	
	FTimerHandle AttackTimerHandle;
	
	bool bIsDead = false;
	bool lifeStealEnabled = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jokers")
	TArray<FName> ActiveJokerEffects;
private:
	void SpawnDamageNumber(AActor* Target, float DamageAmount) const;
};
