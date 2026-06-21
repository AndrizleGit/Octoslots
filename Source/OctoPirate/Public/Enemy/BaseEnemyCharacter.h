#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "BaseEnemyCharacter.generated.h"

UCLASS()
class OCTOPIRATE_API ABaseEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	ABaseEnemyCharacter();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Progression")
	float ExperienceReward = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackRange = 350.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	TSubclassOf<AActor> CoinClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	TSubclassOf<AActor> HealthPackClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drops")
	float HealthPackDropChance = 0.05f;
	
	UFUNCTION(BlueprintCallable, Category = "Enemy|Difficulty")
	void ApplyDifficultyScaling(float HealthMultiplier, float DamageMultiplier);
	
	virtual void OnDeath_Implementation() override;
	
protected:
	virtual void PerformAttack_Implementation() override;
	
private:
	void ChasePlayer();
	
	UPROPERTY()
	ACharacter* PlayerCharacter;
};
