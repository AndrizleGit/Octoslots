#include "Enemy/SuicideCrab.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

ASuicideCrab::ASuicideCrab()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASuicideCrab::BeginPlay()
{
    Super::BeginPlay();
}

void ASuicideCrab::PerformAttack_Implementation()
{
    //do nothing
}

void ASuicideCrab::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsDead || bIsExploding) return;

    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    const float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    if (DistToPlayer <= ExplosionTriggerRange)
    {
        TriggerExplosion();
    }
}

void ASuicideCrab::TriggerExplosion()
{
    if (bIsExploding) return;
    bIsExploding = true;

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);

    if (ExplosionMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(ExplosionMontage);

            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ASuicideCrab::OnExplosionMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, ExplosionMontage);
        }
        else
        {
            Explode();
        }
    }
    else
    {
        Explode();
    }
}

void ASuicideCrab::OnExplosionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    Explode();
}

void ASuicideCrab::Explode()
{
    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ExplosionVFX, GetActorLocation());
    }

    // get all actors in explosion radius
    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), ExplosionRadius, ObjectTypes, nullptr, { this }, OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;

        UGameplayStatics::ApplyDamage(Actor, ExplosionDamage, GetInstigatorController(), this, UDamageType::StaticClass());
    }

    bIsDead = true;
    Destroy();
}

void ASuicideCrab::OnDeath_Implementation()
{
    Super::OnDeath_Implementation();
}