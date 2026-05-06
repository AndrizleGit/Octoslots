// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/OctoPirateAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/OctoPirateEnemyCharacter.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AIPerceptionComponent.h"

AOctoPirateAIController::AOctoPirateAIController(FObjectInitializer const& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
}

void AOctoPirateAIController::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOctoPirateAIController::HandleTargetPerceptionUpdate);
}

void AOctoPirateAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControllingCharacter = Cast<AOctoPirateEnemyCharacter>(InPawn);
	check(ControllingCharacter)
	check(PerceptionComponent)

	UBehaviorTree* const Tree = ControllingCharacter->GetBehaviorTree();

	if (Tree)
	{
		UseBlackboard(Tree->BlackboardAsset, BlackboardComponent);
		Blackboard = BlackboardComponent;
		RunBehaviorTree(Tree);
		Blackboard->SetValueAsBool("CanAttack", true);
	}

	for (UAISenseConfig* SenseConfig : ControllingCharacter->GetSenseConfigs())
	{
		PerceptionComponent->ConfigureSense(*SenseConfig);
	}

	PerceptionComponent->SetDominantSense(ControllingCharacter->GetDominantSense());
	PerceptionComponent->RequestStimuliListenerUpdate();
}

void AOctoPirateAIController::HandleTargetPerceptionUpdate(AActor* Actor, const FAIStimulus Stimulus)
{
	if (!IsValid(ControllingCharacter) || !GetBlackboardComponent()) return;

	ControllingCharacter->HandleSenseUpdate(Actor, Stimulus, GetBlackboardComponent());
}
