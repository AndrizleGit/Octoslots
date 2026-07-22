#include "Upgrades/UpgradeManagerComponent.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "Character/BaseCharacter.h"
#include "OctoSlotsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OctopirateGameInstance.h"

UUpgradeManagerComponent::UUpgradeManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UUpgradeManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    for (UUpgradeData* Upgrade : AvailableUpgrades)
    {
       if (Upgrade)
       {
          UpgradeLevels.Add(Upgrade, 0);
       }
    }
    
    LoadUpgrades(); 
}

void UUpgradeManagerComponent::SaveUpgrades()
{
    UOctoSlotsSaveGame* SaveData = Cast<UOctoSlotsSaveGame>(UGameplayStatics::CreateSaveGameObject(UOctoSlotsSaveGame::StaticClass()));
    if (!SaveData) return;
    
    for (auto& Pair : UpgradeLevels)
    {
       if (Pair.Key) SaveData->UpgradeLevels.Add(Pair.Key->GetName(), Pair.Value);
    }

    SaveData->TotalCoinsSpent = TotalCoinsSpent;
    
    if (bIsMetaProgressionMode)
    {
       UOctopirateGameInstance* GI = Cast<UOctopirateGameInstance>(GetWorld()->GetGameInstance());
       if (GI) SaveData->SavedCoins = GI->TotalMetaCoins;
    }
    else
    {
        UBasicAttributeSet* Attributes = GetPlayerAttributes();
        if (Attributes) SaveData->SavedCoins = Attributes->GetCoins();
    }

    UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);
}

void UUpgradeManagerComponent::LoadUpgrades()
{
    UOctoSlotsSaveGame* SaveData = Cast<UOctoSlotsSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!SaveData) return;

    for (UUpgradeData* Upgrade : AvailableUpgrades)
    {
       if (!Upgrade) continue;
       const int32* SavedLevel = SaveData->UpgradeLevels.Find(Upgrade->GetName());
       if (SavedLevel && *SavedLevel > 0)
       {
          UpgradeLevels[Upgrade] = *SavedLevel;
          if (!bIsMetaProgressionMode) ApplyStatChange(Upgrade->AffectedStat, Upgrade->ValuePerLevel * (*SavedLevel));
       }
    }
    
    if (bIsMetaProgressionMode)
    {
       UOctopirateGameInstance* GI = Cast<UOctopirateGameInstance>(GetWorld()->GetGameInstance());
       if (GI) GI->TotalMetaCoins = SaveData->SavedCoins;
    }
    else
    {
        UBasicAttributeSet* Attributes = GetPlayerAttributes();
        if (Attributes) Attributes->SetCoins(SaveData->SavedCoins);
    }

    TotalCoinsSpent = SaveData->TotalCoinsSpent;
}

bool UUpgradeManagerComponent::PurchaseUpgrade(UUpgradeData* Upgrade)
{
    if (!Upgrade) return false;
    
    if (IsUpgradeMaxLevel(Upgrade)) return false;
    
    UpgradeLevels.FindOrAdd(Upgrade)++;
    ApplyUpgrade(Upgrade);

    SaveUpgrades();
    return true;
}

bool UUpgradeManagerComponent::SellSingleUpgrade(UUpgradeData* Upgrade)
{
    if (!Upgrade) return false;
    
    int32* CurrentLevel = UpgradeLevels.Find(Upgrade);
    if (!CurrentLevel || *CurrentLevel <= 0) 
    {
        return false; 
    }

    (*CurrentLevel)--;
    RemoveUpgrade(Upgrade, 1);

    SaveUpgrades();
    return true;
}

void UUpgradeManagerComponent::RefundAllUpgrades()
{
    float RefundTotal = TotalCoinsSpent;
    
    for (auto& Pair : UpgradeLevels)
    {
       if (Pair.Value > 0)
       {
          if (!bIsMetaProgressionMode) RemoveUpgrade(Pair.Key, Pair.Value);
          Pair.Value = 0;
       }
    }
    
    if (bIsMetaProgressionMode)
    {
        UOctopirateGameInstance* GI = Cast<UOctopirateGameInstance>(GetWorld()->GetGameInstance());
        if (GI) GI->TotalMetaCoins += RefundTotal;
    }
    else
    {
        UBasicAttributeSet* Attributes = GetPlayerAttributes();
        if (Attributes) Attributes->SetCoins(Attributes->GetCoins() + RefundTotal);
    }

    TotalCoinsSpent = 0;
    OnAllUpgradesRefunded.Broadcast();
    SaveUpgrades();
}

void UUpgradeManagerComponent::SelectUpgrade(UUpgradeData* Upgrade)
{
    SelectedUpgrade = Upgrade;
}

int32 UUpgradeManagerComponent::GetUpgradeLevel(UUpgradeData* Upgrade) const
{
    if (!Upgrade) return 0;
    const int32* Level = UpgradeLevels.Find(Upgrade);
    return Level ? *Level : 0;
}

float UUpgradeManagerComponent::GetCurrentStatValue(UUpgradeData* Upgrade) const
{
    if (!Upgrade) return 0.0f;
    const int32 Level = GetUpgradeLevel(Upgrade);
    return Upgrade->ValuePerLevel * Level;
}

float UUpgradeManagerComponent::GetNextStatValue(UUpgradeData* Upgrade) const
{
    if (!Upgrade) return 0.0f;
    const int32 NextLevel = GetUpgradeLevel(Upgrade) + 1;
    return Upgrade->ValuePerLevel * NextLevel;
}

int32 UUpgradeManagerComponent::GetUpgradeCost(UUpgradeData* Upgrade) const
{
    if (!Upgrade) return 0;
    const int32 CurrentLevel = GetUpgradeLevel(Upgrade);
    
    return FMath::RoundToInt(Upgrade->CostPerLevel * FMath::Pow(3.f, CurrentLevel));
}

bool UUpgradeManagerComponent::CanAffordUpgrade(UUpgradeData* Upgrade) const
{
    if (bIsMetaProgressionMode)
    {
        UOctopirateGameInstance* GI = Cast<UOctopirateGameInstance>(GetWorld()->GetGameInstance());
        return GI ? GI->TotalMetaCoins >= GetUpgradeCost(Upgrade) : false;
    }
    else
    {
        UBasicAttributeSet* Attributes = GetPlayerAttributes();
        return Attributes ? Attributes->GetCoins() >= GetUpgradeCost(Upgrade) : false;
    }
}

bool UUpgradeManagerComponent::IsUpgradeMaxLevel(UUpgradeData* Upgrade) const
{
    if (!Upgrade) return false;
    return GetUpgradeLevel(Upgrade) >= Upgrade->MaxLevel;
}

void UUpgradeManagerComponent::ApplyUpgrade(UUpgradeData* Upgrade)
{
    ApplyStatChange(Upgrade->AffectedStat, Upgrade->ValuePerLevel);
}

void UUpgradeManagerComponent::RemoveUpgrade(UUpgradeData* Upgrade, int32 Levels)
{
    ApplyStatChange(Upgrade->AffectedStat, -Upgrade->ValuePerLevel * Levels);
}

void UUpgradeManagerComponent::ApplyStatChange(EUpgradeStat Stat, float Value)
{
    UBasicAttributeSet* Attributes = GetPlayerAttributes();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Attributes || !Character) return;

    switch (Stat)
    {
    case EUpgradeStat::MaxHealth:
       Attributes->SetMaxHealth(Attributes->GetMaxHealth() + Value);
       break;

    case EUpgradeStat::MovementSpeed:
       Attributes->SetWalkSpeed(Attributes->GetWalkSpeed() + Value);
       Character->GetCharacterMovement()->MaxWalkSpeed = Attributes->GetWalkSpeed();
       break;

    case EUpgradeStat::AttackSpeed:
       Attributes->SetAttackSpeed(Attributes->GetAttackSpeed() + Value);
       break;

    case EUpgradeStat::AttackDamage:
       Attributes->SetAttackDamage(Attributes->GetAttackDamage() + Value);
       break;
    }
}

UBasicAttributeSet* UUpgradeManagerComponent::GetPlayerAttributes() const
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) return nullptr;
    return Character->BasicAttributes;
}