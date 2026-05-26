#include "Character/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
//#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Character/AttributeSets/PlayerAttributeSet.h"

// -- Tag Definitions --
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Combat_Hit, "Event.Combat.Hit");

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	
	// -- Ability System --
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	// -- Attribute Sets --
	BasicAttributes = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("BasicAttributeSet"));
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;	
	AttackDamage = GetAttackDamage();
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AbilitySystemComponent)
	{
		// Initialize Actor Info
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// --- Bind Attribute Change Callbacks ---
        
		// Bind Health change
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetHealthAttribute())
			.AddUObject(this, &ABaseCharacter::OnHealthChanged);
		
		// Bind AttackDamage change
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetAttackDamageAttribute())
			.AddUObject(this, &ABaseCharacter::OnAttackDamageChanged);
		// Bind AttackSpeed change
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetAttackSpeedAttribute())
			.AddUObject(this, &ABaseCharacter::OnAttackSpeedChanged);
		
		// Bind WalkSpeed change
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributes->GetWalkSpeedAttribute())
			.AddUObject(this, &ABaseCharacter::OnWalkSpeedChanged);    
	}
	
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ABaseCharacter::PerformAttack_Implementation, GetAttackSpeed(), true);
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
		
		// -- Send OnHitEvent --
		FGameplayEventData Payload;
		Payload.Instigator = GetController();
		Payload.Target = Actor;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor,TAG_Event_Combat_Hit, Payload);
	}

}
// -- Update Attributes -- 
// Update Attributes on Change
void ABaseCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// -- Log the Health change or update a UI/HUD --
	float NewHealth = Data.NewValue;
	float OldHealth = Data.OldValue;
    
	UE_LOG(LogTemp, Warning, TEXT("Health Changed! Old: %f, New: %f"), OldHealth, NewHealth);
    
	
	// -- if Health > MaxHealth , Health = MaxHealth -- 
	if (NewHealth > BasicAttributes->GetMaxHealth()) BasicAttributes->SetHealth( BasicAttributes->GetMaxHealth());
	// -- if Health = 0 -> Death --
	else if (NewHealth <= 0)
	{
		OnDeath();
	}
}
void ABaseCharacter::OnAttackDamageChanged(const FOnAttributeChangeData& Data)
{
	// -- Log the AttackDamage change or update a UI/HUD --
	float NewDamage = Data.NewValue;
	float OldDamage  = Data.OldValue;
    
	UE_LOG(LogTemp, Warning, TEXT("Damage Changed! Old: %f, New: %f"), OldDamage, NewDamage);
    
	AttackDamage = BasicAttributes ? NewDamage : 0.f;
}

void ABaseCharacter::OnAttackSpeedChanged(const FOnAttributeChangeData& Data)
{
	// -- Log the AttackSpeed change or update a UI/HUD --
	float NewAttackSpeed = Data.NewValue;
    
	UE_LOG(LogTemp, Log, TEXT("Attack Speed Updated to: %f"), NewAttackSpeed);

	// If you are using a Timer for attacks (like your AttackTimerHandle), 
	// you need to clear and restart it with the new speed.
	if (GetWorldTimerManager().IsTimerActive(AttackTimerHandle))	
	{
		GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        
		// Calculate new interval (e.g., 1 / AttackSpeed)
		float NewInterval = 1.f / FMath::Max(NewAttackSpeed, 0.01f);
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ABaseCharacter::PerformAttack, NewInterval, true);
	}
}

void ABaseCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	// -- Log the WalkSpeed change or update a UI/HUD --
	float NewWalkSpeed = Data.NewValue;
	float OldWalkSpeed  = Data.OldValue;
    
	UE_LOG(LogTemp, Warning, TEXT("WalkSpeed Changed! Old: %f, New: %f"), OldWalkSpeed, NewWalkSpeed);
    
	GetCharacterMovement()->MaxWalkSpeed = BasicAttributes ? NewWalkSpeed : 400.f;
}

// -- Get Attributes -- 
float ABaseCharacter::GetAttackSpeed() const
{
	const float AttackSpeed = BasicAttributes ? BasicAttributes->GetAttackSpeed() : 1.f;

	return 1.f / FMath::Max(AttackSpeed, 0.01f);
}
float ABaseCharacter::GetAttackDamage() const
{
	const float Health = BasicAttributes ? BasicAttributes->GetAttackDamage() : 15.f;
 
	return Health;
}




float ABaseCharacter::GetWalkSpeed() const
{
	const float WalkSpeed = BasicAttributes ? BasicAttributes->GetWalkSpeed() : 400.f;

	return WalkSpeed;
}

