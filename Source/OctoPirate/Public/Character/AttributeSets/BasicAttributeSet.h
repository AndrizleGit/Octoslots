// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class OCTOPIRATE_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UBasicAttributeSet();
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	
	//Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth)
	
	// Movement Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement")
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, WalkSpeed)
	
	// Combat Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Combat")
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackDamage)
	
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Combat")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackSpeed)
	
	// Pickup Radius Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Pickup")
	FGameplayAttributeData PickupRadius;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, PickupRadius)
	
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Combat")
	FGameplayAttributeData LifeSteal;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, LifeSteal)
	
	// Level Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Level)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Experience)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression|")
	FGameplayAttributeData MaxExperience;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxExperience)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression|")
	FGameplayAttributeData Coins;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Coins)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression|")
	FGameplayAttributeData TotalCoinsCollectedThisRun;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, TotalCoinsCollectedThisRun)
	
public:
	void ApplyLifesteal(float Damage);
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetAttackDamageValue() const { return AttackDamage.GetCurrentValue(); }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetTotalCoinsValue() const { return TotalCoinsCollectedThisRun.GetCurrentValue(); }
};
	