#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"


AOctopusCharacter::AOctopusCharacter()
{
	// --- Tentacle Attack Mesh ---
	TentacleMesh = CreateDefaultSubobject<USkeletalMeshComponent>("TentacleMesh");
	TentacleMesh->SetupAttachment(RootComponent);
	TentacleMesh->SetHiddenInGame(true);
	TentacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// --- Camera ---
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1400.f;
	SpringArm->SetRelativeRotation(FRotator(-55.f, -45.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	UpgradeManager = CreateDefaultSubobject<UUpgradeManagerComponent>("UpgradeManager");
	InRunUpgradeManager = CreateDefaultSubobject<UInRunUpgradeManagerComponent>("InRunUpgradeManager");
}

void AOctopusCharacter::BeginPlay()
{
	Super::BeginPlay();
	BaseConeMaxDistance = ConeMaxDistance;

	// Force absolute rotation & scale AFTER Blueprint init so the BP can't override it
	if (TentacleMesh)
	{
		TentacleMesh->SetUsingAbsoluteRotation(true);
		TentacleMesh->SetUsingAbsoluteScale(true);
	}

	// -- Set Base Attributes --
	BasicAttributes->SetAttackSpeed(1.2f);
}

void AOctopusCharacter::PerformAttack_Implementation()
{
	if (bIsDead) return;

	AActor* ClosestEnemy = GetClosestEnemy();

	if (ClosestEnemy)
	{
		// Check if enemy is within attack range
		const float DistToEnemy = FVector::Dist(GetActorLocation(), ClosestEnemy->GetActorLocation());
		if (DistToEnemy > ConeMaxDistance)
		{
			Super::PerformAttack_Implementation();
			return;
		}

		const FRotator OriginalRotation = GetActorRotation();

		FRotator AttackRotation = (ClosestEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		AttackRotation.Pitch = 0.f;
		AttackRotation.Roll = 0.f;

		SetActorRotation(AttackRotation);
		Super::PerformAttack_Implementation();
		SetActorRotation(OriginalRotation);

		// --- Show tentacle and play attack animation ---
		if (TentacleMesh && TentacleAttackMontage)
		{
			TentacleMesh->SetWorldRotation(AttackRotation + FRotator(0.f, -90.f, 0.f));

			const float RangeRatio = (BaseConeMaxDistance > 0.f) ? ConeMaxDistance / BaseConeMaxDistance : 1.f;
			const float RangeFactor = FMath::Pow(RangeRatio, 5.f); // Very aggressive scaling
			const float FinalScale = BaseTentacleScale * RangeFactor;
			TentacleMesh->SetWorldScale3D(FVector(FinalScale));
			UE_LOG(LogTemp, Warning, TEXT("Tentacle - ConeMax: %f | Base: %f | RangeFactor: %f | BaseTentacleScale: %f | FinalScale: %f"),
				ConeMaxDistance, BaseConeMaxDistance, RangeFactor, BaseTentacleScale, FinalScale);

			TentacleMesh->SetHiddenInGame(false);

			UAnimInstance* AnimInstance = TentacleMesh->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(TentacleAttackMontage);

				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AOctopusCharacter::OnTentacleMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, TentacleAttackMontage);
			}
		}
	}
	else
	{
		Super::PerformAttack_Implementation();
	}
}

void AOctopusCharacter::SetMoveDestination(const FVector& Destination)
{
	AController* MyController = GetController();
	if (!MyController) return;
	
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, Destination);
}

void AOctopusCharacter::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		PC->DisableInput(PC);
		PC->bShowMouseCursor = true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Player has died — Game Over"));
	
	OnPlayerDied.Broadcast();
}

void AOctopusCharacter::OnTentacleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (TentacleMesh)
	{
		TentacleMesh->SetHiddenInGame(true);
		TentacleMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

AActor* AOctopusCharacter::GetClosestEnemy() const
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemyCharacter::StaticClass(), FoundEnemies);

	AActor* Closest     = nullptr;
	float   ClosestDist = FLT_MAX;

	for (AActor* Enemy : FoundEnemies)
	{
		if (!IsValid(Enemy)) continue;

		const float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			Closest     = Enemy;
		}
	}
	
	return Closest;
}


