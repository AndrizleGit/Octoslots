#pragma once

#include "CoreMinimal.h"
#include "SlotMachineTypes.generated.h"

UENUM(BlueprintType)
enum class ESlotSymbol : uint8
{
	MovementSpeed UMETA(DisplayName = "Movement Speed"),
	AttackSpeed UMETA(DisplayName = "Attack Speed"),
	AttackDamage UMETA(DisplayName = "Attack Damage"),
	SEVEN UMETA(DisplayName = "SEVEN"),
	
};

USTRUCT(BlueprintType)
struct FSlotResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly) ESlotSymbol Reel1 = ESlotSymbol::MovementSpeed;
	UPROPERTY(BlueprintReadOnly) ESlotSymbol Reel2 = ESlotSymbol::MovementSpeed;
	UPROPERTY(BlueprintReadOnly) ESlotSymbol Reel3 = ESlotSymbol::MovementSpeed;
	
	int32 GetCount(ESlotSymbol Symbol) const
	{
		int32 Count = 0;
		if (Reel1 == Symbol) Count++;
		if (Reel2 == Symbol) Count++;
		if (Reel3 == Symbol) Count++;
		return Count;
	}
};

USTRUCT(BlueprintType)
struct FSlotBuffConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MovementSpeedBonus = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackSpeedMultiplier = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackDamageBonus = 0.75f;
};