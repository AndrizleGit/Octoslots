// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AI/BTTask_ChasePlayer.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Enemy/OctoPirateAIController.h"

UBTTask_ChasePlayer::UBTTask_ChasePlayer(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AOctoPirateAIController* const AIController = Cast<AOctoPirateAIController>(OwnerComp.GetAIOwner()))
	{
		auto const PlayerLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());
		UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey());
		AActor* TargetActor = Cast<AActor>(TargetObject);

		if (TargetActor)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToActor(AIController, TargetActor);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return EBTNodeResult::Succeeded;
		}

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, PlayerLocation);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
