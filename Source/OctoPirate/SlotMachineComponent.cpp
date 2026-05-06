// Fill out your copyright notice in the Description page of Project Settings.


#include "SlotMachineComponent.h"

#include "OctoPirateCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

USlotMachineComponent::USlotMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USlotMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	DopamineCurrent = DopamineMax;
	PlayerCharacter = Cast<AOctoPirateCharacter>(GetOwner());
}

void USlotMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UE_LOG(LogTemp, Warning, TEXT("Current Movement Speed: %f"), PlayerCharacter->GetMovementComponent()->GetMaxSpeed());
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
	Result.Reel1 = static_cast<ESlotSymbol>(FMath::RandRange(0,2));
	Result.Reel2 = static_cast<ESlotSymbol>(FMath::RandRange(0,2));
	Result.Reel3 = static_cast<ESlotSymbol>(FMath::RandRange(0,2));
	return Result;
}

void USlotMachineComponent::ApplyBuffs(const FSlotResult& Result) const
{
	int32 MovementCount = Result.GetCount(ESlotSymbol::MovementSpeed);
	int32 AttackSpeedCount = Result.GetCount(ESlotSymbol::AttackSpeed);
	int32 AttackDamageCount = Result.GetCount(ESlotSymbol::AttackDamage);
	
	if (MovementCount > 0)
	{
		float Bonus = BuffConfig.MovementSpeedBonus * MovementCount;
		//GetOwner<AOctoPirateCharacter>()->AddMovementSpeedBonus(Bonus);
		if (PlayerCharacter)
		{
			PlayerCharacter->AddMovementSpeedBonus(Bonus);
		}
		UE_LOG(LogTemp, Warning, TEXT("MovementSpeed Tier: %d (+%.1f)"), MovementCount, Bonus);
	}
	
	if (AttackSpeedCount > 0)
	{
		float Multiplier = FMath::Pow(BuffConfig.AttackSpeedMultiplier, AttackSpeedCount);
		//GetOwner<AOctoPirateCharacter>()->ApplyAttackSpeedMultiplier(Multiplier);
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplyAttackSpeedMultiplier(Multiplier);
		}
		UE_LOG(LogTemp, Warning, TEXT("AttackSpeed Tier: %d (+%.1f)"), AttackSpeedCount, Multiplier);
	}
	
	if (AttackDamageCount > 0)
	{
		float Bonus = BuffConfig.AttackDamageBonus * AttackDamageCount;
		//GetOwner<AOctoPirateCharacter>()->AddAttackDamageBonus(Bonus);
		if (PlayerCharacter)
		{
			PlayerCharacter->AddAttackDamageBonus(Bonus);
		}
		UE_LOG(LogTemp, Warning, TEXT("AttackDamage Tier: %d (+%.1f)"), AttackDamageCount, Bonus);
	}
}

void USlotMachineComponent::RemoveAllBuffs() const
{
	//GetOwner<AOctoPirateCharacter>()->ResetSlotMachineBuffs();
	if (PlayerCharacter)
	{
		PlayerCharacter->RemoveMovementSpeedBonus();
		PlayerCharacter->RemoveAttackSpeedMultiplier();
		PlayerCharacter->RemoveAttackDamageBonus();
	}
	UE_LOG(LogTemp, Warning, TEXT("All buffs removed"));
}

void USlotMachineComponent::SetDebuffActive(bool bActive)
{
	if (bDebuffActive == bActive) return;
	
	bDebuffActive = bActive;
	
	if (bActive)
	{
		//GetOwner<AOctoPirateCharacter>()->AddMovementSpeedBonus(DebuffMovementSpeedPenalty);
		if (PlayerCharacter)
		{
			PlayerCharacter->AddMovementSpeedBonus(DebuffMovementSpeedPenalty);
		}
		UE_LOG(LogTemp, Warning, TEXT("Debuff applied"));
	}
	else
	{
		//GetOwner<AOctoPirateCharacter>()->RemoveMovementSpeedBonus();
		if (PlayerCharacter)
		{
			PlayerCharacter->RemoveMovementSpeedBonus();
		}
		UE_LOG(LogTemp, Warning, TEXT("Debuff removed"));
	}
	
	OnDebuffStateChanged.Broadcast(bActive);
}
