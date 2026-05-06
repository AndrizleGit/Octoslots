// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AI/BTTask_FindPlayerLocation.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Find Player Location");
}

EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const ACharacter* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return EBTNodeResult::Failed;

	const FVector PlayerLocation = Player->GetActorLocation();
	if (!SearchRandom)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerLocation);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	FNavLocation Loc;
	const auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	if (NavSys && NavSys->GetRandomPointInNavigableRadius(PlayerLocation, SearchRadius, Loc))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
