#include "Upgrades/InRunUpgradeManagerComponent.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
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
    if (!bIsRunActive) return;

    // Re-entrancy guard: setting the Experience attribute below re-fires the attribute
    // change delegate, which calls back into here synchronously. Bail on the nested call.
    if (bIsProcessingLevelUp) return;

    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    if (!Attributes) return;

    float Experience = Attributes->GetExperience();
    float MaxXP = Attributes->GetMaxExperience();

    if (MaxXP <= 0.f) return;          // misconfigured; avoids divide-by-zero / runaway leveling
    if (Experience < MaxXP) return;    // not enough XP for a level yet

    // Award every level the accumulated XP covers, carrying the remainder into the next level.
    while (Experience >= MaxXP && MaxXP > 0.f)
    {
        Experience -= MaxXP;
        MaxXP *= 1.5f;
        CurrentLevel++;
    }

    // Commit the new XP state under the guard so the resulting change callbacks no-op.
    bIsProcessingLevelUp = true;
    Attributes->SetMaxExperience(MaxXP);
    Attributes->SetExperience(Experience);
    bIsProcessingLevelUp = false;

    RollNewChoices();

    // Only pause for the upgrade screen if there is actually something to choose,
    // otherwise the game would freeze with an empty selection and no way to resume.
    if (CurrentChoices.Num() > 0)
    {
        UGameplayStatics::SetGamePaused(GetWorld(), true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Level up reached but no upgrades are configured; skipping upgrade screen."));
    }

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
    bIsRunActive = true;
    CurrentLevel = 0;
    CurrentChoices.Empty();

    for (auto& Pair : PickedCounts)
        Pair.Value = 0;

    UBasicAttributeSet* Attributes = GetPlayerAttributes();
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
    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Attributes || !Character) return;

    switch (Stat)
    {
        case EInRunUpgradeStat::MaxHealth:
            Attributes->SetMaxHealth(Attributes->GetMaxHealth() + Value);
            Attributes->SetHealth(Attributes->GetHealth() + Value);
            break;

        case EInRunUpgradeStat::MovementSpeed:
            Attributes->SetWalkSpeed(Attributes->GetWalkSpeed() + (Value * 0.5f));
            Character->GetCharacterMovement()->MaxWalkSpeed = Attributes->GetWalkSpeed();
            break;

        case EInRunUpgradeStat::AttackSpeed:
        {
            // +0.15 per pick (e.g. 1.2 → 1.35 → 1.5), cap at 2.5
            const float NewSpeed = FMath::Min(Attributes->GetAttackSpeed() + 0.15f, 2.5f);
            Attributes->SetAttackSpeed(NewSpeed);
            break;
        }

        case EInRunUpgradeStat::AttackDamage:
            Attributes->SetAttackDamage(Attributes->GetAttackDamage() + Value);
            break;

        case EInRunUpgradeStat::AttackRange:
            Character->ConeMaxDistance += Value * 0.5f;
            //Character->ExtraDamageDistance += Value * 0.5f;
            break;
    }
}

UBasicAttributeSet* UInRunUpgradeManagerComponent::GetPlayerAttributes() const
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) return nullptr;
    return Character->BasicAttributes;
}