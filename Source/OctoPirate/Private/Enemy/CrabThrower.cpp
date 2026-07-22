#include "Enemy/CrabThrower.h"
#include "Projectiles/CrabBomb.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ACrabThrower::ACrabThrower()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACrabThrower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead || bIsFrozen) return;
    if (CurrentState == ECrabThrowerState::DiggingUp || CurrentState == ECrabThrowerState::Throwing) return;
    if (!PlayerCharacter) return;

    const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());

    if (CurrentState == ECrabThrowerState::Panicking)
    {
        if (DistToPlayer > PanicRange)
        {
            EnterChasing(); 
        }
        else
        {
            return; // stay panicking
        }
    }

    if (!bHasBomb)
    {
        EnterDiggingUp();
    }
    else if (DistToPlayer <= PanicRange)
    {
        EnterPanicking();
    }
    else if (DistToPlayer <= ThrowRange && !bThrowOnCooldown)
    {
        EnterThrowing();
    }
    else
    {
        MoveTowardPlayer();
    }
}

void ACrabThrower::PerformAttack_Implementation()
{
    // intentionally empty
}

void ACrabThrower::EnterChasing()
{
    CurrentState = ECrabThrowerState::Chasing;
}

void ACrabThrower::EnterDiggingUp()
{
    CurrentState = ECrabThrowerState::DiggingUp;
    GetCharacterMovement()->StopMovementImmediately();

    if (DigUpMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(DigUpMontage);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ACrabThrower::OnDigUpMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, DigUpMontage);
            return;
        }
    }
    OnDigUpMontageEnded(nullptr, false);
}

void ACrabThrower::OnDigUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bHasBomb = true;
    EnterChasing();
}

void ACrabThrower::EnterThrowing()
{
    CurrentState = ECrabThrowerState::Throwing;
    GetCharacterMovement()->StopMovementImmediately();
    
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }

    const FVector ToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    SetActorRotation(ToPlayer.Rotation());

    if (ThrowMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(ThrowMontage);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ACrabThrower::OnThrowMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowMontage);
            return;
        }
    }
    OnThrowMontageEnded(nullptr, false);
}

void ACrabThrower::OnThrowMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        ThrowBomb();
    }
    bHasBomb = false;
    EnterDiggingUp(); // crab always digs up next regardless of what follows
}

void ACrabThrower::ThrowBomb()
{
    if (!BombProjectileClass || !PlayerCharacter) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector SpawnLocation = GetMesh() && GetMesh()->DoesSocketExist(BombThrowSocketName)
        ? GetMesh()->GetSocketLocation(BombThrowSocketName)
        : GetActorLocation() + FVector(0.f, 0.f, 50.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    ACrabBomb* Bomb = World->SpawnActor<ACrabBomb>(BombProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Bomb)
    {
        Bomb->LaunchAtTarget(PlayerCharacter->GetActorLocation(), ThrowArcParam);
    }
    
    PlaySFX(this, ThrowSound, GetActorLocation());

    bThrowOnCooldown = true;
    GetWorldTimerManager().SetTimer(ThrowCooldownTimer, [this]() { bThrowOnCooldown = false; }, ThrowCooldown, false);
}

void ACrabThrower::EnterPanicking()
{
    CurrentState = ECrabThrowerState::Panicking;
    GetCharacterMovement()->StopMovementImmediately();

    if (PanicMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(PanicMontage);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ACrabThrower::OnPanicMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, PanicMontage);
        }
    }
}

void ACrabThrower::OnPanicMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // replay the panic montage as long as we're still in panic state
    if (CurrentState == ECrabThrowerState::Panicking && !bIsDead && !bIsFrozen)
    {
        EnterPanicking();
    }
}

void ACrabThrower::MoveTowardPlayer()
{
    if (bIsDead || bIsFrozen || !PlayerCharacter) return;

    const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
    if (DistToPlayer <= ThrowRange) return;

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->MoveToActor(PlayerCharacter, ThrowRange - 100.f); 
    }
}