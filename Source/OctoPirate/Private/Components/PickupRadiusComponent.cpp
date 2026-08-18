#include "Components/PickupRadiusComponent.h"
#include "Pickups/ABasePickup.h"
#include "Components/SphereComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/AttributeSets/BasicAttributeSet.h"

UPickupRadiusComponent::UPickupRadiusComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPickupRadiusComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SphereCollision = NewObject<USphereComponent>(GetOwner(), TEXT("PickupRadiusSphere"));
    SphereCollision->RegisterComponent();
    SphereCollision->AttachToComponent(GetOwner()->GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character && Character->BasicAttributes)
    {
        SphereCollision->SetSphereRadius(Character->BasicAttributes->GetPickupRadius());
    }
    else
    {
        SphereCollision->SetSphereRadius(200.f);
    }
    
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &UPickupRadiusComponent::OnSphereBeginOverlap);
    SphereCollision->OnComponentEndOverlap.AddDynamic(this, &UPickupRadiusComponent::OnSphereEndOverlap);
}

void UPickupRadiusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    const FVector PlayerLocation = GetOwner()->GetActorLocation();
    const float Radius = SphereCollision ? SphereCollision->GetScaledSphereRadius() : 200.f;

    ActivePickups.RemoveAll([](ABasePickup* Pickup)
    {
        return !IsValid(Pickup);
    });
    
    for (ABasePickup* Pickup : ActivePickups)
    {
        if (ForcedPickups.Contains(Pickup)) continue; 

        const float Dist = FVector::Dist(PlayerLocation, Pickup->GetActorLocation());

        if (Dist <= Radius)
        {
            Pickup->PullToward(PlayerLocation, Pickup->PullSpeed, GetOwner());
        }
        else
        {
            Pickup->StopPull();
        }
    }

    // -- new: magnet-forced pickups, no radius check --
    ForcedPickups.RemoveAll([](ABasePickup* Pickup)
    {
        return !IsValid(Pickup);
    });

    for (ABasePickup* Pickup : ForcedPickups)
    {
        Pickup->PullToward(PlayerLocation, Pickup->PullSpeed, GetOwner());
    }
}

void UPickupRadiusComponent::UpdateRadius(float NewRadius)
{
    if (SphereCollision)
    {
        SphereCollision->SetSphereRadius(NewRadius);
    }
}

void UPickupRadiusComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    ABasePickup* Pickup = Cast<ABasePickup>(OtherActor);
    if (!Pickup) return;
    if (ActivePickups.Contains(Pickup)) return;
    
    ActivePickups.Add(Pickup);
}

void UPickupRadiusComponent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex)
{
    ABasePickup* Pickup = Cast<ABasePickup>(OtherActor);
    if (!Pickup) return;

    ActivePickups.Remove(Pickup);

    if (!ForcedPickups.Contains(Pickup))
    {
        Pickup->StopPull();
    }
}

void UPickupRadiusComponent::ForcePull(ABasePickup* Pickup)
{
    if (!Pickup || ForcedPickups.Contains(Pickup)) return;
    ForcedPickups.Add(Pickup);
}