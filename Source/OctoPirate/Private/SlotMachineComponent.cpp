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

    // drain dopamine over time
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

    // Joker: AutoSpin — automatically spins when player can afford it
    if (PlayerCharacter && PlayerCharacter->HasJokerEffect("AutoSpin"))
    {
        if (AutoSpinCooldownRemaining > 0.f)
        {
            AutoSpinCooldownRemaining -= DeltaTime;
        }
        else if (CanAffordSpin() && !bSpinOnCooldown)
        {
            AutoSpinCooldownRemaining = PlayerCharacter->GetJokerValue("AutoSpin");
            bIsAutoSpinning = true;
            Spin();
            bIsAutoSpinning = false;
        }
    }
}

void USlotMachineComponent::Spin()
{
    if (bSpinOnCooldown)
    {
        UE_LOG(LogTemp, Log, TEXT("Spin blocked — on cooldown"));
        return;
    }
    
    UBasicAttributeSet* Attributes = PlayerCharacter ? PlayerCharacter->BasicAttributes : nullptr;
    if (!Attributes) return;

    if (Attributes->GetCoins() < SpinCost)
    {
        OnSpinFailed.Broadcast();
        return;
    }
    
    bSpinOnCooldown = true;
    Attributes->SetCoins(Attributes->GetCoins() - SpinCost);
    DopamineCurrent = DopamineMax;
    RemoveAllBuffs();
    SetDebuffActive(false);
    LastResult = RollReels();
    ApplyBuffs(LastResult);
    OnSpinComplete.Broadcast(LastResult);
    
    // Joker: Fascinating — freeze nearby enemies on spin
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
    }

    OnDopamineChanged.Broadcast(GetDopamineNormalized());
}

void USlotMachineComponent::OnSpinAnimationFinished()
{
    bSpinOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("Spin animation finished — ready to spin again"));
}

bool USlotMachineComponent::CanAffordSpin() const
{
    const UBasicAttributeSet* Attributes = PlayerCharacter ? PlayerCharacter->BasicAttributes : nullptr;
    return Attributes && Attributes->GetCoins() >= SpinCost;
}

FSlotResult USlotMachineComponent::RollReels()
{
    int max = StaticEnum<ESlotSymbol>()->NumEnums() - 2;
    FSlotResult Result;
    Result.Reel1 = static_cast<ESlotSymbol>(FMath::RandRange(0, max));
    Result.Reel2 = static_cast<ESlotSymbol>(FMath::RandRange(0, max));
    Result.Reel3 = static_cast<ESlotSymbol>(FMath::RandRange(0, max));
    return Result;
}

void USlotMachineComponent::ApplyBuffs(const FSlotResult& Result) const
{
    if (!PlayerCharacter) return;

    // apply stronger buffs when auto-spinning via the AutoSpin joker
    const float Multiplier = bIsAutoSpinning ? AutoSpinBuffMultiplier : 1.0f;

    // three of a kind — unique jackpot effect per symbol
    if (Result.Reel1 == Result.Reel2 && Result.Reel2 == Result.Reel3)
    {
        PlayerCharacter->ApplyThreeOfAKindBuff(Result.Reel1);
        UE_LOG(LogTemp, Warning, TEXT("JACKPOT! Three of a kind: %s"), *UEnum::GetValueAsString(Result.Reel1));
        if (Result.Reel1 == ESlotSymbol::Speed)
        {
            PlayerCharacter->bHelicopterMode = true;
        }
        return;
    }

    const int32 SpeedCount = Result.GetCount(ESlotSymbol::Speed);
    if (SpeedCount > 0)
    {
        PlayerCharacter->ApplySpeedBuff(FMath::RoundToInt(SpeedCount * Multiplier));
        UE_LOG(LogTemp, Warning, TEXT("Speed Tier: %d (Multiplier: %.1f)"), SpeedCount, Multiplier);
    }
    
    const int32 AttackDamageCount = Result.GetCount(ESlotSymbol::AttackDamage);
    if (AttackDamageCount > 0)
    {
        PlayerCharacter->ApplyAttackDamageBuff(FMath::RoundToInt(AttackDamageCount * Multiplier));
        UE_LOG(LogTemp, Warning, TEXT("AttackDamage Tier: %d (Multiplier: %.1f)"), AttackDamageCount, Multiplier);
    }

    const int32 PoisonCount = Result.GetCount(ESlotSymbol::Poison);
    if (PoisonCount > 0)
    {
        PlayerCharacter->ApplyPoisonBuff(FMath::RoundToInt(PoisonCount * Multiplier));
        UE_LOG(LogTemp, Warning, TEXT("Poison Tier: %d (Multiplier: %.1f)"), PoisonCount, Multiplier);
    }

    const int32 LifeStealCount = Result.GetCount(ESlotSymbol::LifeSteal);
    if (LifeStealCount > 0)
    {
        PlayerCharacter->ApplyLifeStealBuff(FMath::RoundToInt(LifeStealCount * Multiplier));
        UE_LOG(LogTemp, Warning, TEXT("LifeSteal Tier: %d (Multiplier: %.1f)"), LifeStealCount, Multiplier);
    }
}

void USlotMachineComponent::RemoveAllBuffs() const
{
    if (PlayerCharacter)
    {
        PlayerCharacter->RemoveBuffs();
         PlayerCharacter->bHelicopterMode = false;
    }
}

void USlotMachineComponent::SetDebuffActive(bool bActive)
{
    if (bDebuffActive == bActive) return;
    
    bDebuffActive = bActive;
    
    if (bActive)
    {
        if (PlayerCharacter) PlayerCharacter->ApplyDebuff();
    }
    else
    {
        if (PlayerCharacter) PlayerCharacter->ClearDebuff();
    }
    
    OnDebuffStateChanged.Broadcast(bActive);
}