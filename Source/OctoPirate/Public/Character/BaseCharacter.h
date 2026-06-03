// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "NativeGameplayTags.h"
#include "BaseCharacter.generated.h"


// -- Tags -- 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Combat_Hit);
	
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
public:	
	virtual void Tick(float DeltaTime) override;
	
	// --- Stats --- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;
	
	// -- Ability System Component --
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributes;
	
	
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
	
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage);
	
	FTimerHandle AttackTimerHandle;
	
	bool bIsDead = false;
	
};
