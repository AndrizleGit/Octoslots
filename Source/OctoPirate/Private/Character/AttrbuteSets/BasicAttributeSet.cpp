// Fill out your copyright notice in the Description page of Project Settings.


#include "Character\AttributeSets\BasicAttributeSet.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	// -- Health --
	Health = 50.0f;
	MaxHealth = 50.0f;
	// -- Movement -- 
	WalkSpeed = 400.0f;
	// --Combat--
	AttackSpeed = 1.0f;
	AttackDamage = 10.0f;
	// --Progression--
	Level = 1.0f;
	Experience = 0.0f;
	MaxExperience = 100.0f;
	Coins = 0.0f;
}

