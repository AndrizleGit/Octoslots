// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/AttributeSets/PlayerAttributeSet.h"

#include "BaseCharacter.generated.h"

UCLASS()
class OCTOPIRATE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	// --- Stats --- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;
	
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
	float AttackDamage = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float ExtraDamage = 45.f;
	
	// --- Functions ---
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealthPercent() const;
	
	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UPlayerAttributeSet* BasicAttributes;
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void PerformAttack();
	virtual void PerformAttack_Implementation();
	
	void ApplyDamageInZone(float MinDist, float MaxDist, float Damage);
	
	FTimerHandle AttackTimerHandle;
	
	bool bIsDead = false;
	
};
