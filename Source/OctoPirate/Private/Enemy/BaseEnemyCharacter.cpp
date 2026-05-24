// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BaseEnemyCharacter.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Character/AttributeSets/PlayerAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TaskSyncManager.h"
#include "Engine/World.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (BasicAttributes)
	{
		GetCharacterMovement()->MaxWalkSpeed = BasicAttributes->GetWalkSpeed();
	}
	
	PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void ABaseEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ChasePlayer();
}

void ABaseEnemyCharacter::ChasePlayer()
{
	if (bIsDead || !PlayerCharacter) return;
	
	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	
	if (DistanceToPlayer <= AttackRange) return;
	
	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), PlayerCharacter);
}

void ABaseEnemyCharacter::PerformAttack_Implementation()
{
	if (bIsDead || !PlayerCharacter) return;
	
	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (DistanceToPlayer > AttackRange) return;
	
	const FVector Direction = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorRotation(Direction.Rotation());
	
	if (BasicAttributes)
	{
		AttackDamage = BasicAttributes->GetAttackDamage();
		AttackInterval = BasicAttributes->GetAttackSpeed();
	}
	
	Super::PerformAttack_Implementation();
}

void ABaseEnemyCharacter::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();
	
	AOctopusCharacter* OctopusCharacter = Cast<AOctopusCharacter>(PlayerCharacter);
	if (OctopusCharacter && OctopusCharacter->BasicAttributes)
	{
		const float CurrentXP = OctopusCharacter->BasicAttributes->GetExperience();
		const float MaxXP = OctopusCharacter->BasicAttributes->GetMaxExperience();
		OctopusCharacter->BasicAttributes->SetExperience(FMath::Min(CurrentXP + ExperienceReward, MaxXP));
	}
	
	SetLifeSpan(2.f);
}
