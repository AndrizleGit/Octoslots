#include "Enemy/Shark.h"

#include "AIController.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Character/BaseCharacter.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AShark::AShark()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AShark::BeginPlay()
{
    Super::BeginPlay();
}

void AShark::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead || bIsFrozen || !PlayerCharacter) return;

    if (ChargeState == ESharkChargeState::Charging)
    {
        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            AICon->StopMovement();
        }
        TickCharge(DeltaTime);
        return;
    }

    if (ChargeState == ESharkChargeState::Telegraphing)
    {
        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            AICon->StopMovement(); // re-assert every tick — cancels whatever the BP just issued
        }
        return;
    }
    
    if (!bChargeOnCooldown)
    {
        const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
        TryBeginTelegraph(DistToPlayer);
    }
}

void AShark::TryBeginTelegraph(float DistToPlayer)
{
    if (DistToPlayer <= ChargeTriggerRange)
    {
        BeginTelegraph();
    }
}

void AShark::BeginTelegraph()
{
    ChargeState = ESharkChargeState::Telegraphing;
    SetMovementLocked(true);
    GetCharacterMovement()->StopMovementImmediately();

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }
    
    ChargeDirection = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    ChargeDirection.Z = 0.f;
    ChargeStartLocation = GetActorLocation();

    SetActorRotation(ChargeDirection.Rotation());

    if (LaneDecalMaterial)
    {
        const FVector DecalLocation = GetActorLocation() + ChargeDirection * (ChargeDistance * 0.5f);
        FRotator DecalRotation = ChargeDirection.Rotation();
        DecalRotation.Pitch -= 90.f; // project straight down -- a purely horizontal decal never lands on the floor
        // X = projection depth, Y = half lane width, Z = half lane length
        const FVector DecalSize = FVector(200.f, ChargeLaneWidth * 0.5f, ChargeDistance * 0.5f);

        ActiveLaneDecal = UGameplayStatics::SpawnDecalAtLocation(this, LaneDecalMaterial, DecalSize, DecalLocation, DecalRotation, TelegraphDuration);
    }

    ABaseCharacter::PlaySFX(this, TelegraphSound, GetActorLocation());

    GetWorldTimerManager().SetTimer(TelegraphTimer, this, &AShark::BeginCharge, TelegraphDuration, false);
}

void AShark::BeginCharge()
{
    ClearLaneDecal();
    ChargeState = ESharkChargeState::Charging;
    bHasHitPlayerThisCharge = false;

    ABaseCharacter::PlaySFX(this, ChargeSound, GetActorLocation());
}

void AShark::TickCharge(float DeltaTime)
{
    const FVector Delta = ChargeDirection * ChargeSpeed * DeltaTime;
    AddActorWorldOffset(Delta, true);

    if (!bHasHitPlayerThisCharge)
    {
        const FVector ToPlayer = PlayerCharacter->GetActorLocation() - GetActorLocation();
        const float LateralDist = FVector::CrossProduct(ChargeDirection, ToPlayer).Size();
        const float ForwardDist = FVector::DotProduct(ChargeDirection, ToPlayer);

        if (LateralDist <= ChargeLaneWidth * 0.5f && FMath::Abs(ForwardDist) <= 100.f)
        {
            UGameplayStatics::ApplyDamage(PlayerCharacter, ChargeDamage, GetController(), this, UDamageType::StaticClass());
            bHasHitPlayerThisCharge = true;
        }
    }

    const float TraveledDist = FVector::Dist(GetActorLocation(), ChargeStartLocation);
    if (TraveledDist >= ChargeDistance)
    {
        EndCharge();
    }
}

void AShark::EndCharge()
{
    ChargeState = ESharkChargeState::None;
    SetMovementLocked(false);
    bChargeOnCooldown = true;
    GetWorldTimerManager().SetTimer(ChargeCooldownTimer, [this]() { bChargeOnCooldown = false; }, ChargeCooldown, false);
}

void AShark::ClearLaneDecal()
{
    if (ActiveLaneDecal)
    {
        ActiveLaneDecal->DestroyComponent();
        ActiveLaneDecal = nullptr;
    }
}