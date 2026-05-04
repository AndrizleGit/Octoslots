#pragma once

#include "CoreMinimal.h"
#include "SlotMachineTypes.generated.h"

UENUM(BlueprintType)
enum class ESlotSymbol : uint8
{
	MovementSpeed UMETA(DisplayName = "Movement Speed"),
	AttackSpeed UMETA(DisplayName = "Attack Speed"),
	AttackDamage UMETA(DisplayName = "Attack Damage"),
};

USTRUCT(BlueprintType)
struct FSlotResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly) int32 MovementSpeedCount = 0;
	UPROPERTY(BlueprintReadOnly) int32 AttackSpeedCount = 0;
	UPROPERTY(BlueprintReadOnly) int32 AttackDamageCount = 0;
};

USTRUCT(BlueprintType)
struct FSlotBuffConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MovementSpeedBonus = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackSpeedMultiplier = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackDamageBonus = 10.f;
};