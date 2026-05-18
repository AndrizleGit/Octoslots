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
	
	//Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth)
	
	// Movement Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes")
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, WalkSpeed)
	
	
	// Attack Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Atrributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackSpeed)

	
};
