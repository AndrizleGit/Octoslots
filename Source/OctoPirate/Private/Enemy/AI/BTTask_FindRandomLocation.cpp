// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AI/BTTask_FindRandomLocation.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/OctoPirateAIController.h"
#include "Enemy/OctoPirateEnemyCharacter.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = "Find Random Location In NavMesh";
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOctoPirateAIController* const AIController = Cast<AOctoPirateAIController>(OwnerComp.GetAIOwner());
	check(AIController)

	const APawn* const EnemyPawn = AIController->GetPawn();
	check(EnemyPawn)

	FVector const Origin = EnemyPawn->GetActorLocation();

	const auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLocation;

	if (NavSys && NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, NavLocation))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), NavLocation.Location);
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
