#include "Upgrades/InRunUpgradeManagerComponent.h"
#include "Character/AttributeSets/PlayerAttributeSet.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UInRunUpgradeManagerComponent::UInRunUpgradeManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInRunUpgradeManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize pick counts for all upgrades
    for (UInRunUpgradeData* Upgrade : AllPossibleUpgrades)
    {
        if (Upgrade)
            PickedCounts.Add(Upgrade, 0);
    }
}

void UInRunUpgradeManagerComponent::CheckForLevelUp()
{
    UPlayerAttributeSet* Attributes = GetPlayerAttributes();
    if (!Attributes) return;
    
    if (Attributes->GetExperience() < Attributes->GetMaxExperience()) return;
    
    CurrentLevel++;
    
    const float NewMaxXP = Attributes->GetMaxExperience() * 1.5f;
    Attributes->SetExperience(0.0f);
    Attributes->SetMaxExperience(NewMaxXP);

    RollNewChoices();

    UGameplayStatics::SetGamePaused(GetWorld(), true);

    OnLevelUp.Broadcast(CurrentLevel);
}

void UInRunUpgradeManagerComponent::RollNewChoices()
{
    CurrentChoices.Empty();

    TArray<UInRunUpgradeData*> AvailablePool;
    for (UInRunUpgradeData* Upgrade : AllPossibleUpgrades)
    {
        if (Upgrade)
            AvailablePool.Add(Upgrade);
    }

    const int32 ChoiceCount = FMath::Min(3, AvailablePool.Num());
    for (int32 i = 0; i < ChoiceCount; i++)
    {
        const int32 RandomIndex = FMath::RandRange(0, AvailablePool.Num() - 1);
        CurrentChoices.Add(AvailablePool[RandomIndex]);
        AvailablePool.RemoveAt(RandomIndex); 
    }
}

void UInRunUpgradeManagerComponent::SelectUpgrade(UInRunUpgradeData* Upgrade)
{
    if (!Upgrade) return;

    ApplyUpgrade(Upgrade);

    if (PickedCounts.Contains(Upgrade))
        PickedCounts[Upgrade]++;

    UGameplayStatics::SetGamePaused(GetWorld(), false);

    OnUpgradeSelected.Broadcast(Upgrade);

    UE_LOG(LogTemp, Log, TEXT("In-run upgrade selected: %s"), *Upgrade->UpgradeName.ToString());
}

void UInRunUpgradeManagerComponent::ResetForNewRun()
{
    CurrentLevel = 0;
    CurrentChoices.Empty();

    for (auto& Pair : PickedCounts)
        Pair.Value = 0;

    UPlayerAttributeSet* Attributes = GetPlayerAttributes();
    if (Attributes)
    {
        Attributes->SetExperience(0.0f);
        Attributes->SetMaxExperience(100.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("In-run upgrades reset for new run"));
}

void UInRunUpgradeManagerComponent::ApplyUpgrade(UInRunUpgradeData* Upgrade)
{
    ApplyStatChange(Upgrade->AffectedStat, Upgrade->UpgradeValue);
}

void UInRunUpgradeManagerComponent::ApplyStatChange(EInRunUpgradeStat Stat, float Value)
{
    UPlayerAttributeSet* Attributes = GetPlayerAttributes();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Attributes || !Character) return;

    switch (Stat)
    {
        case EInRunUpgradeStat::MaxHealth:
            Attributes->SetMaxHealth(Attributes->GetMaxHealth() + Value);
            Attributes->SetHealth(Attributes->GetHealth() + Value);
            break;

        case EInRunUpgradeStat::MovementSpeed:
            Attributes->SetWalkSpeed(Attributes->GetWalkSpeed() + Value);
            Character->GetCharacterMovement()->MaxWalkSpeed = Attributes->GetWalkSpeed();
            break;

        case EInRunUpgradeStat::AttackSpeed:
            Attributes->SetAttackSpeed(Attributes->GetAttackSpeed() + Value);
            break;

        case EInRunUpgradeStat::AttackDamage:
            Attributes->SetAttackDamage(Attributes->GetAttackDamage() + Value);
            break;

        case EInRunUpgradeStat::AttackRange:
            Character->ConeMaxDistance += Value * 0.5f;
            //Character->ExtraDamageDistance += Value * 0.5f;
            break;
    }
}

UPlayerAttributeSet* UInRunUpgradeManagerComponent::GetPlayerAttributes() const
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) return nullptr;
    return Character->BasicAttributes;
}