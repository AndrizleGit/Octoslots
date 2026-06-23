#include "SlotMachineComponent.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
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
	UBasicAttributeSet* Attributes = PlayerCharacter ? PlayerCharacter->BasicAttributes : nullptr;
	if (!Attributes) return;

	// Charge for the spin. Refuse and notify if the player can't afford it.
	if (Attributes->GetCoins() < SpinCost)
	{
		OnSpinFailed.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Spin denied: costs %.0f coins, player has %.0f"), SpinCost, Attributes->GetCoins());
		return;
	}
	Attributes->SetCoins(Attributes->GetCoins() - SpinCost);

	DopamineCurrent = DopamineMax;

	RemoveAllBuffs();
	SetDebuffActive(false);

	LastResult = RollReels();
	ApplyBuffs(LastResult);

	OnSpinComplete.Broadcast(LastResult);
	
	// -- Joker: Fascinating --
	if (PlayerCharacter && PlayerCharacter->HasJokerEffect("Fascinating"))
	{
		const float FreezeDuration = PlayerCharacter->GetJokerValue("Fascinating");
		const float FreezeRadius = 1500.f;

		TArray<AActor*> NearbyEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemyCharacter::StaticClass(), NearbyEnemies);

		for (AActor* Actor : NearbyEnemies)
		{
			ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Actor);
			if (!Enemy) continue;

			const float Dist = FVector::Dist(PlayerCharacter->GetActorLocation(), Enemy->GetActorLocation());
			if (Dist <= FreezeRadius)
			{
				Enemy->Freeze(FreezeDuration);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Fascinating — froze %d nearby enemies for %.1f seconds"), NearbyEnemies.Num(), FreezeDuration);
	}
	OnDopamineChanged.Broadcast(GetDopamineNormalized());
}

bool USlotMachineComponent::CanAffordSpin() const
{
	const UBasicAttributeSet* Attributes = PlayerCharacter ? PlayerCharacter->BasicAttributes : nullptr;
	return Attributes && Attributes->GetCoins() >= SpinCost;
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
	if (!PlayerCharacter) return;

	// Three of a kind: with only three reels, a match means every reel shows the same
	// symbol, so no other symbol can be present. The unique jackpot buff therefore
	// naturally replaces any stacking buff.
	if (Result.Reel1 == Result.Reel2 && Result.Reel2 == Result.Reel3)
	{
		PlayerCharacter->ApplyThreeOfAKindBuff(Result.Reel1);
		UE_LOG(LogTemp, Warning, TEXT("JACKPOT! Three of a kind: %s"), *UEnum::GetValueAsString(Result.Reel1));
		return;
	}

	// Otherwise apply tiered stacking buffs for any symbol that appears once or twice.
	const int32 MovementCount = Result.GetCount(ESlotSymbol::MovementSpeed);
	if (MovementCount > 0)
	{
		PlayerCharacter->ApplyMovementSpeedBuff(MovementCount);
		UE_LOG(LogTemp, Warning, TEXT("MovementSpeed Tier: %d"), MovementCount);
	}

	const int32 AttackSpeedCount = Result.GetCount(ESlotSymbol::AttackSpeed);
	if (AttackSpeedCount > 0)
	{
		PlayerCharacter->ApplyAttackSpeedBuff(AttackSpeedCount);
		UE_LOG(LogTemp, Warning, TEXT("AttackSpeed Tier: %d"), AttackSpeedCount);
	}

	const int32 AttackDamageCount = Result.GetCount(ESlotSymbol::AttackDamage);
	if (AttackDamageCount > 0)
	{
		PlayerCharacter->ApplyAttackDamageBuff(AttackDamageCount);
		UE_LOG(LogTemp, Warning, TEXT("AttackDamage Tier: %d"), AttackDamageCount);
	}

	const int32 PoisonCount = Result.GetCount(ESlotSymbol::Poison);
	if (PoisonCount > 0)
	{
		PlayerCharacter->ApplyPoisonBuff(PoisonCount);
		UE_LOG(LogTemp, Warning, TEXT("Poison Tier: %d"), PoisonCount);
	}

	const int32 LifeStealCount = Result.GetCount(ESlotSymbol::LifeSteal);
	if (LifeStealCount > 0)
	{
		PlayerCharacter->ApplyLifeStealBuff(LifeStealCount);
		UE_LOG(LogTemp, Warning, TEXT("LifeSteal Tier: %d"), LifeStealCount);
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
