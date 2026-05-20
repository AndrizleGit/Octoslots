#include "Character//BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;	
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ABaseCharacter::PerformAttack_Implementation, AttackInterval, true);
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                 AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;
	
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	
	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath();
	}
	
	return ActualDamage;
}

void ABaseCharacter::OnDeath_Implementation()
{
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	
	//Override this for custom death
}

float ABaseCharacter::GetHealthPercent() const
{
	return CurrentHealth / MaxHealth;
}

void ABaseCharacter::PerformAttack_Implementation()
{
	if (bIsDead) return;
	
	ApplyDamageInZone(0.0f, ExtraDamageDistance, AttackDamage);
	ApplyDamageInZone(ExtraDamageDistance,ConeMaxDistance, ExtraDamage);
	
#if ENABLE_DRAW_DEBUG
	const FVector Origin  = GetActorLocation();
	FVector Forward       = GetActorForwardVector();
	Forward.Z             = 0.0f;
	Forward.Normalize();

	const float HalfAngle = ConeAngleDegrees * 0.5f;
	const float DebugZ    = Origin.Z;
	const float Duration  = 0.15f;
	const int32 Segments  = 16;

	for (int32 i = 0; i <= Segments; i++)
	{
		const float Angle1 = FMath::DegreesToRadians(-HalfAngle + (i - 1) * ConeAngleDegrees / Segments);
		const float Angle2 = FMath::DegreesToRadians(-HalfAngle + i * ConeAngleDegrees / Segments);

		FVector Near1 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle1), 0).RotateVector(Forward) * ExtraDamageDistance;
		FVector Near2 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle2), 0).RotateVector(Forward) * ExtraDamageDistance;
		Near1.Z = Near2.Z = DebugZ;
		DrawDebugLine(GetWorld(), Near1, Near2, FColor::Yellow, false, Duration, 0, 2.0f);

		FVector Far1 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle1), 0).RotateVector(Forward) * ConeMaxDistance;
		FVector Far2 = Origin + FRotator(0, FMath::RadiansToDegrees(Angle2), 0).RotateVector(Forward) * ConeMaxDistance;
		Far1.Z = Far2.Z = DebugZ;
		DrawDebugLine(GetWorld(), Far1, Far2, FColor::Red, false, Duration, 0, 2.0f);
	}

	const FVector LeftDir  = FRotator(0,  HalfAngle, 0).RotateVector(Forward);
	const FVector RightDir = FRotator(0, -HalfAngle, 0).RotateVector(Forward);
	FVector LeftEnd  = Origin + LeftDir  * ConeMaxDistance; LeftEnd.Z  = DebugZ;
	FVector RightEnd = Origin + RightDir * ConeMaxDistance; RightEnd.Z = DebugZ;
	DrawDebugLine(GetWorld(), FVector(Origin.X, Origin.Y, DebugZ), LeftEnd,  FColor::Red, false, Duration, 0, 2.0f);
	DrawDebugLine(GetWorld(), FVector(Origin.X, Origin.Y, DebugZ), RightEnd, FColor::Red, false, Duration, 0, 2.0f);
#endif
}

void ABaseCharacter::ApplyDamageInZone(float MinDist, float MaxDist, float Damage)
{
	if (bIsDead) return;
	
	const FVector Origin = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
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
		ToTarget.Z = 0.0f;
		const float Distance = ToTarget.Size();
		
		if (Distance < MinDist || Distance > MaxDist) continue;
		
		const float DotProduct = FVector::DotProduct(Forward, ToTarget.GetSafeNormal());
		const float AngleToTarget = FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f));
		
		if (AngleToTarget > HalfAngleRad) continue;
		
		UGameplayStatics::ApplyDamage(Actor, Damage, GetController(), this, UDamageType::StaticClass());
	}

}

float AOctopusCharacter::GetAttackSpeed() const
{
	const float AttackSpeed = BasicAttributes ? BasicAttributes->GetAttackSpeed() : 1.f;
