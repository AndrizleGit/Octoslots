// Fill out your copyright notice in the Description page of Project Settings.


#include "SlotMachineComponent.h"

#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

USlotMachineComponent::USlotMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USlotMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	DopamineCurrent = DopamineMax;
		PlayerCharacter = Cast<AOctopusCharacter>(GetOwner());
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
	int max = StaticEnum<ESlotSymbol>()->NumEnums() - 2; // -2 to exclude the auto-generated _MAX entry
	FSlotResult Result;
	Result.Reel1 = static_cast<ESlotSymbol>(FMath::RandRange(0,max));
	Result.Reel2 = static_cast<ESlotSymbol>(FMath::RandRange(0,max));
	Result.Reel3 = static_cast<ESlotSymbol>(FMath::RandRange(0,max));
	return Result;
}

void USlotMachineComponent::ApplyBuffs(const FSlotResult& Result) const
{
	int32 MovementCount = Result.GetCount(ESlotSymbol::MovementSpeed);
	int32 AttackSpeedCount = Result.GetCount(ESlotSymbol::AttackSpeed);
	int32 AttackDamageCount = Result.GetCount(ESlotSymbol::AttackDamage);
	int32 SevenCount = Result.GetCount(ESlotSymbol::SEVEN);
	if (MovementCount > 0)
	{
		
		//--- Call ApplyMovementSpeedBuff(StackCount)  ---
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplyMovementSpeedBuff(MovementCount);
		}
		UE_LOG(LogTemp, Warning, TEXT("MovementSpeed Tier: %d "), MovementCount);
	}
	
	if (AttackSpeedCount > 0)
	{
		
		// --- Call ApplyAttackSpeedBuff(StackCount) ---
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplyAttackSpeedBuff(AttackSpeedCount);
		}
		UE_LOG(LogTemp, Warning, TEXT("AttackSpeed Tier: %d"), AttackSpeedCount);
	}
	
	if (AttackDamageCount > 0)
	{
		// --- Call ApplyAttackDamageBuff(StackCount) ---
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplyAttackDamageBuff(AttackDamageCount);
		}
		UE_LOG(LogTemp, Warning, TEXT("AttackDamage Tier: %d"), AttackDamageCount );
	}
	if (SevenCount == 3)
	{
		// --- Call ApplySevenBuff() ---
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplySevenBuff();
		}
		UE_LOG(LogTemp, Warning, TEXT("GOD MODE!"));
	}

}

void USlotMachineComponent::RemoveAllBuffs() const
{
	//---  Call ClearBuffs() ---
	if (PlayerCharacter)
	{
		PlayerCharacter->RemoveBuffs();
	}
	UE_LOG(LogTemp, Warning, TEXT("All buffs removed"));
}

void USlotMachineComponent::SetDebuffActive(bool bActive)
{
	if (bDebuffActive == bActive) return;
	
	bDebuffActive = bActive;
	
	if (bActive)
	{
		// -- Apply Debuffs --
		if (PlayerCharacter)
		{
			PlayerCharacter->ApplyDebuff();
		}
		UE_LOG(LogTemp, Warning, TEXT("Debuff applied"));
	}
	else
	{
		// -- Clear Debuffs --
		if (PlayerCharacter)
		{
			PlayerCharacter->ClearDebuff();
		}
		UE_LOG(LogTemp, Warning, TEXT("Debuff removed"));
	}
	
	OnDebuffStateChanged.Broadcast(bActive);
}
