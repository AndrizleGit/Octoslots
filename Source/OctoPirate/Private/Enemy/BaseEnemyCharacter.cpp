// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BaseEnemyCharacter.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TaskSyncManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	// -- Set Base Enemy Attributes --
	if (BasicAttributes)
	{
		BasicAttributes->SetAttackDamage(7.f);
		BasicAttributes->SetWalkSpeed(BasicAttributes->GetWalkSpeed() * 1.2f); // 20% faster than base
		GetCharacterMovement()->MaxWalkSpeed = BasicAttributes->GetWalkSpeed();
	}
	
	PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
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
	if (bIsDead || !PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Attack skipped (Dead: %d, NoPlayer: %d)"), *GetName(), bIsDead, !PlayerCharacter);
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (DistanceToPlayer > AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Out of range (Dist: %.1f, Range: %.1f)"), *GetName(), DistanceToPlayer, AttackRange);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Enemy %s: ATTACKING player (Dist: %.1f, Dmg: %.1f)"), *GetName(), DistanceToPlayer, AttackDamage);

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
	
	const FVector SpawnLocation = GetActorLocation();
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (CoinClass)
	{
		GetWorld()->SpawnActor<AActor>(CoinClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	if (HealthPackClass)
	{
		const float Roll = FMath::RandRange(0.0f, 1.0f);
		if (Roll <= HealthPackDropChance)
		{
			const FVector HealthPackLocation = SpawnLocation + FVector(30.f, 30.f, 0.f);
			GetWorld()->SpawnActor<AActor>(HealthPackClass, HealthPackLocation, SpawnRotation, SpawnParams);
		}
	}
	GetMesh()->SetVisibility(false);
	SetLifeSpan(2.f);
}
