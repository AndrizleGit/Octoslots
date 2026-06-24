#include "Upgrades/InRunUpgradeManagerComponent.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "Character/BaseCharacter.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UInRunUpgradeManagerComponent::UInRunUpgradeManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInRunUpgradeManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // initialize pick counts for all upgrades
    for (UInRunUpgradeData* Upgrade : AllPossibleUpgrades)
    {
        if (Upgrade)
            PickedCounts.Add(Upgrade, 0);
    }
}

void UInRunUpgradeManagerComponent::CheckForLevelUp()
{
    if (!bIsRunActive) return;
    if (bIsProcessingLevelUp) return;

    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    if (!Attributes) return;

    float Experience = Attributes->GetExperience();
    float MaxXP = Attributes->GetMaxExperience();

    if (MaxXP <= 0.f) return;
    if (Experience < MaxXP) return;

    while (Experience >= MaxXP && MaxXP > 0.f)
    {
        Experience -= MaxXP;
        MaxXP *= 1.5f;
        CurrentLevel++;
    }

    // lock against re-entry while writing XP attributes
    bIsProcessingLevelUp = true;
    Attributes->SetMaxExperience(MaxXP);
    Attributes->SetExperience(Experience);
    bIsProcessingLevelUp = false;

    const bool bIsJokerLevel = (JokerLevelInterval > 0) && (CurrentLevel % JokerLevelInterval == 0);
    const bool bCanGetJoker = AcquiredJokers.Num() < MaxJokers && AllPossibleJokers.Num() > 0;

    if (bIsJokerLevel && bCanGetJoker)
    {
        RollNewJokerChoices();
        if (CurrentJokerChoices.Num() > 0)
        {
            UGameplayStatics::SetGamePaused(GetWorld(), true);
            OnJokerLevelUp.Broadcast(CurrentLevel);
        }
    }
    else
    {
        RollNewChoices();
        if (CurrentChoices.Num() > 0)
        {
            UGameplayStatics::SetGamePaused(GetWorld(), true);
            OnLevelUp.Broadcast(CurrentLevel);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Level up to %d but no upgrades configured — skipping pause"), CurrentLevel);
        }
    }
}

// picks 3 random upgrades from the pool without duplicates
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

// picks 3 random jokers, excluding ones already acquired this run
void UInRunUpgradeManagerComponent::RollNewJokerChoices()
{
    CurrentJokerChoices.Empty();

    TArray<UJokerData*> AvailablePool;
    for (UJokerData* Joker : AllPossibleJokers)
    {
        if (Joker && !AcquiredJokers.Contains(Joker))
            AvailablePool.Add(Joker);
    }

    const int32 ChoiceCount = FMath::Min(3, AvailablePool.Num());
    for (int32 i = 0; i < ChoiceCount; i++)
    {
        const int32 RandomIndex = FMath::RandRange(0, AvailablePool.Num() - 1);
        CurrentJokerChoices.Add(AvailablePool[RandomIndex]);
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

void UInRunUpgradeManagerComponent::SelectJoker(UJokerData* Joker)
{
    if (!Joker) return;

    AcquiredJokers.Add(Joker);

    // register the effect on the character so HasJokerEffect() checks work everywhere
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        Character->AddJokerEffect(Joker->JokerEffectID, Joker->JokerValue);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), false);
    OnJokerSelected.Broadcast(Joker);
    UE_LOG(LogTemp, Log, TEXT("Joker selected: %s (Effect: %s, Value: %.1f)"),
        *Joker->JokerName.ToString(), *Joker->JokerEffectID.ToString(), Joker->JokerValue);
}

void UInRunUpgradeManagerComponent::CaptureBaseline()
{
    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Attributes || !Character) return;

    BaselineMaxHealth       = Attributes->GetMaxHealth();
    BaselineWalkSpeed       = Attributes->GetWalkSpeed();
    BaselineAttackSpeed     = Attributes->GetAttackSpeed();
    BaselineAttackDamage    = Attributes->GetAttackDamage();
    BaselineConeMaxDistance = Character->ConeMaxDistance;
    bBaselineCaptured = true;

    UE_LOG(LogTemp, Log, TEXT("In-run stat baseline captured (HP %.1f, Dmg %.1f, AtkSpd %.2f, Walk %.0f, Cone %.0f)"),
        BaselineMaxHealth, BaselineAttackDamage, BaselineAttackSpeed, BaselineWalkSpeed, BaselineConeMaxDistance);
}

void UInRunUpgradeManagerComponent::ResetForNewRun()
{
    bIsRunActive = true;
    CurrentLevel = 0;
    CurrentChoices.Empty();
    CurrentJokerChoices.Empty();
    AcquiredJokers.Empty();

    for (auto& Pair : PickedCounts)
        Pair.Value = 0;

    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Attributes && Character)
    {
        // restore stats (undoing all in-run upgrades)
        if (bBaselineCaptured)
        {
            Attributes->SetMaxHealth(BaselineMaxHealth);
            Attributes->SetHealth(BaselineMaxHealth);
            Attributes->SetWalkSpeed(BaselineWalkSpeed);
            Attributes->SetAttackSpeed(BaselineAttackSpeed);
            Attributes->SetAttackDamage(BaselineAttackDamage);
            Character->ConeMaxDistance = BaselineConeMaxDistance;
            Character->GetCharacterMovement()->MaxWalkSpeed = BaselineWalkSpeed;
        }

        Attributes->SetExperience(0.0f);
        Attributes->SetMaxExperience(100.0f);

        // joker effect IDs live on the character, not just this component
        Character->ClearAllJokerEffects();
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
            break;
    }
}

UBasicAttributeSet* UInRunUpgradeManagerComponent::GetPlayerAttributes() const
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) return nullptr;
    return Character->BasicAttributes;
}