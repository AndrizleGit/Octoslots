// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "OctoPirateEnemyCharacter.generated.h"

class UBlackboardComponent;
struct FAIStimulus;
class UAISense;
class UAISenseConfig;

DECLARE_MULTICAST_DELEGATE(FOnEnemyDied)

UCLASS()
class OCTOPIRATE_API AOctoPirateEnemyCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

private:

	UPROPERTY(EditAnywhere, Instanced, Category="AI|Perception")
	TArray<UAISenseConfig*> SenseConfigs = {};

	UPROPERTY(EditAnywhere, Category="AI|Perception")
	TSubclassOf<UAISense> DominantSense = nullptr;

	UPROPERTY()
	UBlackboardComponent* CachedBlackboardComponent = nullptr;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MoveSpeed = 300.f;

	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void HandleCharacterDeath(AActor* DeathInstigator);

public:

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE const TArray<UAISenseConfig*>& GetSenseConfigs() const { return SenseConfigs; }
	FORCEINLINE TSubclassOf<UAISense> GetDominantSense() const { return DominantSense; }

	void HandleSenseUpdate(AActor* Actor, const FAIStimulus& Stimulus, UBlackboardComponent* BlackboardComp);

	FOnEnemyDied OnEnemyDied;
};
