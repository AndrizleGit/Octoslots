#include "Character/OctopusCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Character/AttributeSets/PlayerAttributeSet.h"

AOctopusCharacter::AOctopusCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	
	//--- Camera Tuning ---
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

	// -- Ability System --
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	// -- Attribute Sets --
	BasicAttributes = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("BasicAttributeSet"));
}

void AOctopusCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AOctopusCharacter::PerformAttack, GetAttackSpeed(), true);
}

void AOctopusCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AOctopusCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AOctopusCharacter::SetMoveDestination(const FVector& Destination)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;
	
	AController* MyController = GetController();
	if (!MyController) return;
	
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, Destination);
}

void AOctopusCharacter::PerformAttack()
{
	ApplyDamageInZone(0.f, SwordStartDistance, TentacleDamage);
	
	ApplyDamageInZone(SwordStartDistance, ConeMaxDistance,SwordDamage);
	
#if ENABLE_DRAW_DEBUG
	const FVector Origin = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	
	const float HalfAngle = ConeAngleDegrees * 0.5f;
	const float DebugZ = Origin.Z;
	const float Duration = 0.15;
	const int32 Segments = 16;
	
	for (int32 i = 0; i < Segments; i++)
	{
		const float Angle1 = FMath::DegreesToRadians(-HalfAngle + (i - 1) * ConeAngleDegrees / Segments);
		const float Angle2 = FMath::DegreesToRadians(-HalfAngle + i * ConeAngleDegrees / Segments);
		
		FVector Near1 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle1), 0).RotateVector(Forward) * SwordStartDistance;
		FVector Near2 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle2), 0).RotateVector(Forward) * SwordStartDistance;
		Near1.Z = DebugZ;
		Near2.Z = DebugZ;
		DrawDebugLine(GetWorld(), Near1, Near2, FColor::Yellow, false, Duration, 0, 2.0f);

		FVector Far1 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle1), 0).RotateVector(Forward) * ConeMaxDistance;
		FVector Far2 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle2), 0).RotateVector(Forward) * ConeMaxDistance;
		Far1.Z = DebugZ;
		Far2.Z = DebugZ;
		DrawDebugLine(GetWorld(), Far1, Far2, FColor::Red, false, Duration, 0, 2.0f);
	}
	
	const FVector LeftDir  = FRotator(0,  HalfAngle, 0).RotateVector(Forward);
	const FVector RightDir = FRotator(0, -HalfAngle, 0).RotateVector(Forward);

	FVector LeftEnd  = Origin + LeftDir  * ConeMaxDistance; LeftEnd.Z  = DebugZ;
	FVector RightEnd = Origin + RightDir * ConeMaxDistance; RightEnd.Z = DebugZ;

	DrawDebugLine(GetWorld(), FVector(Origin.X, Origin.Y, DebugZ), LeftEnd,  FColor::Red, false, Duration, 0, 2.0f);
	DrawDebugLine(GetWorld(), FVector(Origin.X, Origin.Y, DebugZ), RightEnd, FColor::Red, false, Duration, 0, 2.0f);

	for (int32 i = 0; i <= Segments; i++)
	{
		const float Angle1 = FMath::DegreesToRadians(-HalfAngle + (i - 1) * ConeAngleDegrees / Segments);
		const float Angle2 = FMath::DegreesToRadians(-HalfAngle + i       * ConeAngleDegrees / Segments);

		FVector Mid1 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle1), 0).RotateVector(Forward) * SwordStartDistance;
		FVector Mid2 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle2), 0).RotateVector(Forward) * SwordStartDistance;
		Mid1.Z = DebugZ;
		Mid2.Z = DebugZ;
		DrawDebugLine(GetWorld(), Mid1, Mid2, FColor::Yellow, false, Duration, 0, 2.0f);
	}
#endif
}

void AOctopusCharacter::ApplyDamageInZone(float MinDist, float MaxDist, float Damage)
{
	const FVector Origin = GetActorLocation();
	
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	
	const float HalfAngleRad = FMath::DegreesToRadians(ConeAngleDegrees * 0.5f);
	
	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Origin, MaxDist, ObjectTypes, nullptr, { this }, OverlappingActors);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor) continue;
		
		FVector ToTarget = Actor->GetActorLocation() - Origin;
		ToTarget.Z = 0.f;
		const float Distance = ToTarget.Size();
		
		if (Distance < MinDist || Distance > MaxDist) continue;
		
		const float DotProduct = FVector::DotProduct(Forward, ToTarget.GetSafeNormal());
		const float AngleToTarget = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
		
		if (AngleToTarget > HalfAngleRad) continue;
		
		UGameplayStatics::ApplyDamage(Actor, Damage, GetController(), this, UDamageType::StaticClass());
	}

}

float AOctopusCharacter::GetAttackSpeed() const
{
	const float AttackSpeed = BasicAttributes ? BasicAttributes->GetAttackSpeed() : 1.f;

	return 1.f / FMath::Max(AttackSpeed, 0.01f);
}

void AOctopusCharacter::PossessedBy(AController* NewController)
	{
		Super::PossessedBy(NewController);
		if (AbilitySystemComponent)
		{
			// Initialize Actor Info
			AbilitySystemComponent->InitAbilityActorInfo(this, this);

			// --- Bind Attribute Change Callbacks ---
        
			// Bind Health change
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetHealthAttribute())
				.AddUObject(this, &AOctopusCharacter::OnHealthChanged);

			// Bind AttackSpeed change
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetAttackSpeedAttribute())
				.AddUObject(this, &AOctopusCharacter::OnAttackSpeedChanged);
            
		}
}

UAbilitySystemComponent* AOctopusCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
// Update Attributes on Change
void AOctopusCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// Example: Log the health change or update a UI/HUD
	float NewHealth = Data.NewValue;
	float OldHealth = Data.OldValue;
    
	UE_LOG(LogTemp, Warning, TEXT("Health Changed! Old: %f, New: %f"), OldHealth, NewHealth);
    
	// Logic for Death, UI Updates, or VFX could go here
	if (NewHealth <= 0)
	{
		// Handle Death
	}
}

void AOctopusCharacter::OnAttackSpeedChanged(const FOnAttributeChangeData& Data)
{
	float NewAttackSpeed = Data.NewValue;
    
	UE_LOG(LogTemp, Log, TEXT("Attack Speed Updated to: %f"), NewAttackSpeed);

	// If you are using a Timer for attacks (like your AttackTimerHandle), 
	// you need to clear and restart it with the new speed.
	if (GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        
		// Calculate new interval (e.g., 1 / AttackSpeed)
		float NewInterval = 1.f / FMath::Max(NewAttackSpeed, 0.01f);
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AOctopusCharacter::PerformAttack, NewInterval, true);
	}
}
