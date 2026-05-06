// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/OctoPirateEnemyCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/OctoPirateAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig_Sight.h"

AOctoPirateEnemyCharacter::AOctoPirateEnemyCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	Health = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SenseConfigs.Add(SightConfig);
	DominantSense = UAISenseConfig_Sight::StaticClass();

	AIControllerClass = AOctoPirateAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AOctoPirateEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

float AOctoPirateEnemyCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= ActualDamage;

	if (Health <= 0.f)
	{
		HandleCharacterDeath(DamageCauser);
	}

	return ActualDamage;
}

void AOctoPirateEnemyCharacter::HandleCharacterDeath(AActor* DeathInstigator)
{
	OnEnemyDied.Broadcast();
	Destroy();
}

void AOctoPirateEnemyCharacter::HandleSenseUpdate(AActor* Actor, const FAIStimulus& Stimulus, UBlackboardComponent* BlackboardComp)
{
	if (!Actor || !BlackboardComp) return;

	static const FAISenseID SightId = UAISense::GetSenseID<UAISenseConfig_Sight>();

	if (Stimulus.Type == SightId)
	{
		CachedBlackboardComponent = BlackboardComp;
		BlackboardComp->SetValueAsBool(TEXT("CanSeePlayer"), Stimulus.WasSuccessfullySensed());

		if (Stimulus.WasSuccessfullySensed())
		{
			BlackboardComp->SetValueAsObject(TEXT("TargetPlayer"), Actor);
		}
		else
		{
			BlackboardComp->SetValueAsObject(TEXT("TargetPlayer"), nullptr);
		}
	}
}
