// Fill out your copyright notice in the Description page of Project Settings.


#include "SlotMachineComponent.h"

#include "OctoPirateCharacter.h"

USlotMachineComponent::USlotMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USlotMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	DopamineCurrent = DopamineMax;
}

void USlotMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (DopamineCurrent > 0.f)
	{
		DopamineCurrent = FMath::Max(0.f, DopamineCurrent - (DopamineDrainPerSecond * DeltaTime));
		OnDopamineChanged.Broadcast(GetDopamineNormalized());
		
		if (DopamineCurrent <= 0.f)
		{
			RemoveAllBuffs();
			SetDebuffActive(true);
			OnDopamineEmpty.Broadcast();
		}
	}
}

void USlotMachineComponent::Spin()
{
	DopamineCurrent = DopamineMax;
	
	RemoveAllBuffs();
	SetDebuffActive(false);
	
	LastResult = RollReels();
	ApplyBuffs(LastResult);
	
	OnSpinComplete.Broadcast(LastResult);
	OnDopamineChanged.Broadcast(GetDopamineNormalized());
}

FSlotResult USlotMachineComponent::RollReels()
{
	FSlotResult Result;
	
	for (int32 i = 0; i < 3; i++)
	{
		switch (ESlotSymbol RolledSymbol = static_cast<ESlotSymbol>(FMath::RandRange(0,2)))
		{
			case ESlotSymbol::MovementSpeed: Result.MovementSpeedCount++; break;
			case ESlotSymbol::AttackSpeed: Result.AttackSpeedCount++; break;
			case ESlotSymbol::AttackDamage: Result.AttackDamageCount++; break;
		}
	}
	
	return Result;
}

void USlotMachineComponent::ApplyBuffs(const FSlotResult& Result) const
{
	if (Result.MovementSpeedCount > 0)
	{
		float Bonus = BuffConfig.MovementSpeedBonus * Result.MovementSpeedCount;
		//GetOwner<AOctoPirateCharacter>()->AddMovementSpeedBonus(Bonus);
		UE_LOG(LogTemp, Warning, TEXT("MovementSpeed Tier: %d (+%.1f)"), Result.MovementSpeedCount, Bonus);
	}
	
	if (Result.AttackDamageCount > 0)
	{
		float Multiplier = FMath::Pow(BuffConfig.AttackSpeedMultiplier, Result.AttackSpeedCount);
		//GetOwner<AOctoPirateCharacter>()->ApplyAttackSpeedMultiplier(Multiplier);
		UE_LOG(LogTemp, Warning, TEXT("AttackSpeed Tier: %d (+%.1f)"), Result.AttackSpeedCount, Multiplier);
	}
	
	if (Result.MovementSpeedCount > 0)
	{
		float Bonus = BuffConfig.AttackDamageBonus * Result.AttackDamageCount;
		//GetOwner<AOctoPirateCharacter>()->AddAttackDamageBonus(Bonus);
		UE_LOG(LogTemp, Warning, TEXT("AttackDamage Tier: %d (+%.1f)"), Result.AttackDamageCount, Bonus);
	}
}

void USlotMachineComponent::RemoveAllBuffs()
{
	//GetOwner<AOctoPirateCharacter>()->ResetSlotMachineBuffs();
	UE_LOG(LogTemp, Warning, TEXT("All buffs removed"));
}

void USlotMachineComponent::SetDebuffActive(bool bActive)
{
	if (bDebuffActive == bActive) return;
	
	bDebuffActive = bActive;
	
	if (bActive)
	{
		//GetOwner<AOctoPirateCharacter>()->AddMovementSpeedBonus(DebuffMovementSpeedPenalty);
		UE_LOG(LogTemp, Warning, TEXT("Debuff applied"));
	}
	else
	{
		//GetOwner<AOctoPirateCharacter>()->RemoveMovementSpeedBonus();
		UE_LOG(LogTemp, Warning, TEXT("Debuff removed"));
	}
	
	OnDebuffStateChanged.Broadcast(bActive);
}
