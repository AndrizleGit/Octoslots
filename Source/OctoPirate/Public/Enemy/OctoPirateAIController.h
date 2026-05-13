// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OctoPirateAIController.generated.h"

struct FAIStimulus;
class AOctoPirateEnemyCharacter;

UCLASS()
class OCTOPIRATE_API AOctoPirateAIController : public AAIController
{
	GENERATED_UCLASS_BODY()

private:

	UPROPERTY(Transient)
	AOctoPirateEnemyCharacter* ControllingCharacter;

	UPROPERTY()
	class UBlackboardComponent* BlackboardComponent;

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void HandleTargetPerceptionUpdate(AActor* Actor, const FAIStimulus Stimulus);

public:

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
};
