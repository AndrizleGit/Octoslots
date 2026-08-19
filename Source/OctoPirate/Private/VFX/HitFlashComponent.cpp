// Fill out your copyright notice in the Description page of Project Settings.

#include "VFX/HitFlashComponent.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// These must match the parameter names inside M_HitFlash_Overlay exactly.
	// A typo here fails silently -- no warning, the flash just never appears.
	const FName ParamAmount(TEXT("HitFlashAmount"));
	const FName ParamColor(TEXT("HitFlashColor"));

	const TCHAR* DefaultFlashMaterialPath =
		TEXT("/Game/Materials/VFX_Materials/HitFlash/MI_HitFlash_Overlay.MI_HitFlash_Overlay");
}

UHitFlashComponent::UHitFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Only ticks while a flash is actually running -- with a horde on screen
	// this is the difference between one tick and two hundred idle ones.
	PrimaryComponentTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFlashMaterial(
		DefaultFlashMaterialPath);
	if (DefaultFlashMaterial.Succeeded())
	{
		FlashMaterial = DefaultFlashMaterial.Object;
	}
}

void UHitFlashComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!FlashMaterial)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("HitFlashComponent on %s has no FlashMaterial -- run "
			     "Content/Python/create_hit_flash_material.py to build it."),
			*GetNameSafe(GetOwner()));
		return;
	}

	TArray<UMeshComponent*> Meshes;
	GetOwner()->GetComponents<UMeshComponent>(Meshes);
	if (Meshes.Num() == 0)
	{
		return;
	}

	FlashMeshes.Reserve(Meshes.Num());
	for (UMeshComponent* Mesh : Meshes)
	{
		FlashMeshes.Add(Mesh);
	}

	// One MID shared across every mesh on the actor -- they always flash
	// together, so there is nothing to gain from one each.
	FlashMID = UMaterialInstanceDynamic::Create(FlashMaterial, this);
	if (FlashMID)
	{
		FlashMID->SetVectorParameterValue(ParamColor, FlashColor);
		FlashMID->SetScalarParameterValue(ParamAmount, 0.f);
	}
}

void UHitFlashComponent::Flash()
{
	if (Duration <= 0.f || !FlashMID || FlashMeshes.Num() == 0)
	{
		return;
	}

	const bool bWasIdle = (TimeRemaining <= 0.f);

	TimeRemaining = Duration;
	SetAmount(1.f);

	if (bWasIdle)
	{
		ApplyOverlay(FlashMID);
		SetComponentTickEnabled(true);
	}
}

void UHitFlashComponent::StopFlash()
{
	if (TimeRemaining <= 0.f)
	{
		return;
	}

	TimeRemaining = 0.f;
	SetAmount(0.f);
	ApplyOverlay(nullptr);
	SetComponentTickEnabled(false);
}

void UHitFlashComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeRemaining -= DeltaTime;
	if (TimeRemaining <= 0.f)
	{
		TimeRemaining = 0.f;
		SetAmount(0.f);
		ApplyOverlay(nullptr);
		SetComponentTickEnabled(false);
		return;
	}

	// Hold at full for HoldFraction of the duration, then ramp down to 0. The
	// hold is what makes the hit land -- a straight fade reads as a glow.
	const float Alpha = TimeRemaining / Duration;   // 1 -> 0
	const float Falloff = FMath::Max(1.f - HoldFraction, KINDA_SMALL_NUMBER);
	SetAmount(FMath::Min(Alpha / Falloff, 1.f));
}

void UHitFlashComponent::SetAmount(float Amount)
{
	if (FlashMID)
	{
		FlashMID->SetScalarParameterValue(ParamAmount, Amount);
	}
}

void UHitFlashComponent::ApplyOverlay(UMaterialInterface* Material)
{
	for (UMeshComponent* Mesh : FlashMeshes)
	{
		if (Mesh)
		{
			Mesh->SetOverlayMaterial(Material);
		}
	}
}
