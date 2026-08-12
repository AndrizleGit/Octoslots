#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Projectiles/BaseProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BaseCharacter.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "VFX/BombActor.h"
#include "Engine/World.h"

// -- Tag Definition --
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_PoisonImmune, "Status.PoisonImmune")
UE_DEFINE_GAMEPLAY_TAG(TAG_Debuffs_PlayerPoison,     "Debuffs.PlayerPoison")
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_PoisonWeaponBuff,     "Buffs.PoisonWeapon")
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_PoisonTrailBuff,     "Buffs.PoisonTrail")
UE_DEFINE_GAMEPLAY_TAG(TAG_Debuffs_PoisonTrailDebuff,     "Debuffs.PoisonTrail")
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
	
	PickupRadius = CreateDefaultSubobject<UPickupRadiusComponent>("PickupRadius");
	
	// --- range decal ---
	AttackRangeDecal = CreateDefaultSubobject<UDecalComponent>("AttackRangeDecal");
	AttackRangeDecal->SetupAttachment(RootComponent);
	// X is the projection half-depth. The decal sits on the capsule origin (88 units up),
	// so it needs enough depth to reach the ground and follow slopes.
	AttackRangeDecal->DecalSize = FVector(400.f, 100.f, 100.f);
	AttackRangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void AOctopusCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Force absolute rotation & scale AFTER Blueprint init so the BP can't override it
	if (TentacleMesh)
	{
		TentacleMesh->SetUsingAbsoluteRotation(true);
		TentacleMesh->SetUsingAbsoluteScale(true);
	}

	// -- Set Base Attributes --
	BasicAttributes->SetAttackSpeed(1.2f);
	
	// -- Init Ability SpecHandle --
	if (AbilitySystemComponent && PoisonEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = 
			AbilitySystemComponent->MakeEffectContext();
            
		CachedPoisonSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			PoisonEffectClass,
			1.0f,
			ContextHandle
		);
		if (PoisonPathEffectClass)
		{
			ContextHandle = 
				AbilitySystemComponent->MakeEffectContext();
            
			CachedPoisonPathSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
				PoisonPathEffectClass,
				1.0f,
				ContextHandle
		
			);
		}
	}
	// -- Checking for Poison Trail Tag -- 
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_PoisonTrailBuff, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &AOctopusCharacter::OnTagChanged);

	// Snapshot the post-init stats (constructor defaults + meta-progression applied during
	// the components' BeginPlay, plus the base AttackSpeed set above) as the per-run baseline.
	// ResetForNewRun restores exactly this, so runs no longer inherit the previous run's upgrades.
	if (InRunUpgradeManager)
	{
		InRunUpgradeManager->CaptureBaseline();
	}
	
	if (AttackRangeDecal)
	{
		AttackRangeDecal->DecalSize = FVector(400.f, ConeMaxDistance, ConeMaxDistance);
	}
}

void AOctopusCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCanDeflect && CooldownRemaining > 0.f)
	{
		CooldownRemaining -= DeltaTime;
		OnDeflectCooldownChanged.Broadcast(1.f - (CooldownRemaining / DeflectCooldown));

		if (CooldownRemaining <= 0.f)
		{
			CooldownRemaining = 0.f;
			bCanDeflect = true;
			OnDeflectCooldownChanged.Broadcast(1.f);
		}
	}
}

void AOctopusCharacter::PerformAttack_Implementation()
{
    if (bIsDead) return;

    // -- Joker: Scrooge --
    float EffectiveDamage = AttackDamage;
    if (HasJokerEffect("Scrooge") && BasicAttributes)
    {
        const float MinCoins = GetJokerValue("Scrooge");
        const float CurrentCoins = BasicAttributes->GetCoins();

        if (CurrentCoins >= MinCoins)
        {
            EffectiveDamage += CurrentCoins * ScroogeDamagePerCoin;
            UE_LOG(LogTemp, Log, TEXT("Scrooge — Coins: %.0f, Bonus: %.1f, Total: %.1f"),
                CurrentCoins, CurrentCoins * ScroogeDamagePerCoin, EffectiveDamage);
        }
    }

    AActor* ClosestEnemy = GetClosestEnemy();

    if (ClosestEnemy)
    {
        const float DistToEnemy = FVector::Dist(GetActorLocation(), ClosestEnemy->GetActorLocation());

        // Trigger slightly early
        if (DistToEnemy > ConeMaxDistance)
        {
            return; // nothing close enough
        }

        const FRotator OriginalRotation = GetActorRotation();
        FRotator AttackRotation = (ClosestEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
        AttackRotation.Pitch = 0.f;
        AttackRotation.Roll = 0.f;
        SetActorRotation(AttackRotation);

        ApplyDamageInZone(0.0f, ConeMaxDistance, EffectiveDamage);
        
        //PlaySFX(this, AttackSound, GetActorLocation());

        SetActorRotation(OriginalRotation);

        // --- Show tentacle and play attack animation ---
        if (TentacleMesh && TentacleAttackMontage)
        {
            TentacleMesh->SetWorldRotation(AttackRotation + FRotator(0.f, TentacleYawOffset, 0.f));
            TentacleMesh->SetWorldScale3D(FVector(1.f));

            TentacleMesh->SetHiddenInGame(false);

            if (!TentacleMesh->GetSkeletalMeshAsset())
            {
                UE_LOG(LogTemp, Warning, TEXT("[Tentacle] TentacleMesh has no Skeletal Mesh asset assigned in BP_OctopusCharacter — nothing will render."));
            }

            UAnimInstance* AnimInstance = TentacleMesh->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(TentacleAttackMontage);

                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &AOctopusCharacter::OnTentacleMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, TentacleAttackMontage);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Tentacle] TentacleMesh->GetAnimInstance() is null — set the component's Anim Class (Anim Blueprint) in BP_OctopusCharacter, or the montage cannot play."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Tentacle] Skipped: TentacleMesh=%s, TentacleAttackMontage=%s. Assign the montage in BP_OctopusCharacter (Combat|Tentacle)."),
                TentacleMesh ? TEXT("OK") : TEXT("NULL"),
                TentacleAttackMontage ? TEXT("OK") : TEXT("NULL"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("[Tentacle] No closest enemy — attack skipped this cycle."));
        return;
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
	
	PlaySFX(this, DeathSound, GetActorLocation());
	
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

void AOctopusCharacter::ApplyDamageInZone(float MinDist, float MaxDist, float Damage)
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
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel2));
	
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

        // -- Knockback --
        ACharacter* HitCharacter = Cast<ACharacter>(Actor);
        if (HitCharacter)
        {
            const FVector KnockbackDirection = ToTarget.GetSafeNormal();
            HitCharacter->LaunchCharacter(KnockbackDirection * KnockbackStrength, true, false);
        }
    	
    	SpawnDamageNumber(Actor, Damage);
    	
        // -- Lifesteal --
        if (lifeStealEnabled) BasicAttributes->ApplyLifesteal(Damage);

        // -- Apply Poison --
        UAbilitySystemComponent* TargetASC = 
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

        if (!TargetASC) continue;
        
        if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_PoisonWeaponBuff))
        {
            if (!CachedPoisonSpecHandle.IsValid()) continue;
            if (TargetASC->HasMatchingGameplayTag(TAG_Status_PoisonImmune)) continue;
            if (TargetASC->HasMatchingGameplayTag(TAG_Debuffs_PlayerPoison)) continue;

            for (int i = 0; i < GetStacksByTag(AbilitySystemComponent, TAG_Status_PoisonWeaponBuff); i++)
            {
                AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*CachedPoisonSpecHandle.Data.Get(), TargetASC);
            }
        }

        // -- Apply 3 Kind Poison --
        if (TargetASC->HasMatchingGameplayTag(TAG_Debuffs_PoisonTrailDebuff)) continue;
        if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_PoisonTrailBuff))
        {
            if (!CachedPoisonPathSpecHandle.IsValid()) continue;
            AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*CachedPoisonPathSpecHandle.Data.Get(), TargetASC);
        }
    }
}

int32 AOctopusCharacter::GetStacksByTag(UAbilitySystemComponent* ASC, FGameplayTag EffectTag)
{
	if (!ASC) return 0;

	   FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(
        FGameplayTagContainer(EffectTag)
	);

	TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffects(Query);

	int32 TotalStacks = 0;
	for (const FActiveGameplayEffectHandle& Handle : Handles)
	{
		TotalStacks += ASC->GetCurrentStackCount(Handle);
	}

	return TotalStacks;
}

// -- Joker: Bomb --

void AOctopusCharacter::OnJokerEffectAdded(FName EffectID, float Value)
{
	Super::OnJokerEffectAdded(EffectID, Value);
	UE_LOG(LogTemp, Warning, TEXT("[Joker] OnJokerEffectAdded: '%s' (value %.1f)"), *EffectID.ToString(), Value);
	RefreshBombTimer();
}

void AOctopusCharacter::OnJokerEffectRemoved(FName EffectID)
{
	Super::OnJokerEffectRemoved(EffectID);
	RefreshBombTimer();
}

void AOctopusCharacter::RefreshBombTimer()
{
	const bool bHasJoker = HasJokerEffect("BombDrop");
	const bool bWant = bHasJoker && BombClass != nullptr && BombSpawnInterval > 0.f;
	const bool bActive = GetWorldTimerManager().IsTimerActive(BombSpawnTimer);

	UE_LOG(LogTemp, Warning, TEXT("[Bomb] RefreshBombTimer: HasJoker=%d BombClassSet=%d Interval=%.1f -> want=%d (alreadyActive=%d)"),
		bHasJoker, BombClass != nullptr, BombSpawnInterval, bWant, bActive);

	if (bHasJoker && !BombClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Bomb] BombDrop joker is active but BombClass is NOT set on BP_OctopusCharacter — no bombs will spawn."));
	}

	if (bWant && !bActive)
	{
		GetWorldTimerManager().SetTimer(BombSpawnTimer, this, &AOctopusCharacter::SpawnBombBehind, BombSpawnInterval, true);
	}
	else if (!bWant && bActive)
	{
		GetWorldTimerManager().ClearTimer(BombSpawnTimer);
	}
}

void AOctopusCharacter::SpawnBombBehind()
{
	if (bIsDead || !BombClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector SpawnLocation = GetActorLocation() - GetActorForwardVector() * BombSpawnDistanceBehind;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	ABombActor* Bomb = World->SpawnActor<ABombActor>(BombClass, SpawnLocation, GetActorRotation(), SpawnParams);
	UE_LOG(LogTemp, Warning, TEXT("[Bomb] SpawnBombBehind at %s -> %s"), *SpawnLocation.ToString(), Bomb ? TEXT("spawned") : TEXT("FAILED"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, FString::Printf(TEXT("Bomb dropped: %s"), Bomb ? TEXT("OK") : TEXT("FAILED")));
	}
}

void AOctopusCharacter::GrantJoker(FName EffectID, float Value)
{
	// Console cheat for testing: `GrantJoker BombDrop` or `GrantJoker DeathExplosion`
	AddJokerEffect(EffectID, Value);
	UE_LOG(LogTemp, Warning, TEXT("[Joker] GrantJoker cheat: granted '%s'"), *EffectID.ToString());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Granted joker: %s"), *EffectID.ToString()));
	}
}

void AOctopusCharacter::SetDeflectActive(bool bActive)
{
	if (bActive == bDeflectActive) return;
	bDeflectActive = bActive;

	if (bActive)
	{
		OnDeflectStarted();
	}
	else
	{
		OnDeflectEnded();
	}
}

void AOctopusCharacter::TriggerDeflect()
{
	if (!bCanDeflect || bDeflectActive) return;

	// start cooldown instantly
	bCanDeflect = false;
	CooldownRemaining = DeflectCooldown;
	OnDeflectCooldownChanged.Broadcast(0.f);

	SetDeflectActive(true);
	
	PlaySFX(this, DeflectSound, GetActorLocation());

	// deactivate deflect window after short time
	GetWorldTimerManager().SetTimer(
		DeflectTimer,
		[this]()
		{
			SetDeflectActive(false);
		},
		0.5f,
		false
	);
}

void AOctopusCharacter::DeactivateDeflect()
{
	SetDeflectActive(false);
}
