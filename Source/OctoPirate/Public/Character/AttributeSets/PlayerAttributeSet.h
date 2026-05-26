// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class OCTOPIRATE_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	UPlayerAttributeSet();
	
	//Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, MaxHealth)
	
	// Movement Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement")
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, WalkSpeed)
	
	// Attack Damage Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Combat")
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, AttackDamage)
	
	// Attack Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes|Combat")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, AttackSpeed)
	
	// Level Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, Level)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, Experience)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression|")
	FGameplayAttributeData MaxExperience;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, MaxExperience)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Progression|")
	FGameplayAttributeData Coins;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, Coins)
};
	