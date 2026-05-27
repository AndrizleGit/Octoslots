#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"


AOctopusCharacter::AOctopusCharacter()
{
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
	// -- Set Base Attributes -- 
	BasicAttributes->SetAttackSpeed(1.2f);	
}

void AOctopusCharacter::PerformAttack_Implementation()
{
	if (bIsDead) return;
	
	AActor* ClosestEnemy = GetClosestEnemy();
	
	if (ClosestEnemy)
	{
		const FRotator OriginalRotation = GetActorRotation();
		
		FRotator AttackRotation = (ClosestEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		AttackRotation.Pitch = 0.f;
		AttackRotation.Roll = 0.f;
		
		SetActorRotation(AttackRotation);
		Super::PerformAttack_Implementation();
		SetActorRotation(OriginalRotation);
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


