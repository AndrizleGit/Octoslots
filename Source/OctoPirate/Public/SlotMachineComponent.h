// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "SlotMachineTypes.h"
#include "SlotMachineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpinComplete, FSlotResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDopamineChanged, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDopamineEmpty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDebuffStateChanged, bool, bIsDebuffed);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OCTOPIRATE_API USlotMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USlotMachineComponent();

	// --- Config ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Dopamine")
	float DopamineMax = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Dopamine")
	float DopamineDrainPerSecond = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Buff")
	FSlotBuffConfig BuffConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Machine|Debuff")
	float DebuffMovementSpeedPenalty = -200.f;
	
	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category="Slot Machine|Events")
	FOnSpinComplete OnSpinComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
	FOnDopamineChanged  OnDopamineChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
	FOnDopamineEmpty  OnDopamineEmpty;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Machine|Events")
	FOnDebuffStateChanged OnDebuffStateChanged;
	
	// --- Public API ---
	UFUNCTION(BlueprintCallable, Category = "Slot Machine")
	void Spin();
	
	UFUNCTION(BlueprintPure, Category = "Slot Machine")
	float GetDopamineNormalized() const { return DopamineCurrent / DopamineMax; }

	UFUNCTION(BlueprintPure, Category = "Slot Machine")
	bool IsDebuffActive() const { return bDebuffActive; }
	
	UFUNCTION(BlueprintPure, Category = "Slot Machine")
	FSlotResult GetLastResult() const { return LastResult; }
	
	// --- Overrides ---
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	UPROPERTY()
	AOctopusCharacter* PlayerCharacter = nullptr;
	float DopamineCurrent = 100.f;
	bool bDebuffActive = false;
	FSlotResult LastResult;

	static FSlotResult RollReels();
	void ApplyBuffs(const FSlotResult& Result) const;
	void RemoveAllBuffs() const;
	void SetDebuffActive(bool bActive);
};
