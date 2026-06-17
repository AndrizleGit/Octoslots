#include "Character/PlayerCharacter/OctopusPlayerController.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"

#define TRACE_GROUND ECC_GameTraceChannel1

AOctopusPlayerController::AOctopusPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	bEnableMouseOverEvents = true;
}

void AOctopusPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (OctopusMappingContext)
		{
			Subsystem->AddMappingContext(OctopusMappingContext, 0);
		}
	}
	

}

void AOctopusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(RightClickAction, ETriggerEvent::Started, this, &AOctopusPlayerController::OnRightMousePressed);
		EnhancedInput->BindAction(RightClickAction, ETriggerEvent::Completed, this, &AOctopusPlayerController::OnRightMouseReleased);
	}
}

void AOctopusPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	if (bRightMouseHeld)
	{
		MoveToCursor();
	}
}

void AOctopusPlayerController::OnRightMousePressed()
{
	bRightMouseHeld = true;
	MoveToCursor();
	SpawnCursorFX();
}

void AOctopusPlayerController::OnRightMouseReleased()
{
	bRightMouseHeld = false;
}

void AOctopusPlayerController::MoveToCursor() const
{
	FHitResult HitResult;

	bool bHit = GetHitResultUnderCursor(TRACE_GROUND, false, HitResult);

	if (!bHit) return;
	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s at %s"),
			*HitResult.GetActor()->GetName(),
			*HitResult.ImpactPoint.ToString());
	}
	AOctopusCharacter* OctopusChar = Cast<AOctopusCharacter>(GetPawn());
	if (!OctopusChar) return;

	OctopusChar->SetMoveDestination(HitResult.ImpactPoint);
}

void AOctopusPlayerController::SpawnCursorFX()
{
	if (!CursorClickFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursorClickFX is not assigned!"));
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("CursorFX: No hit under cursor"));
		return;
	}

	const FVector CursorLocation = HitResult.ImpactPoint;

	UE_LOG(LogTemp, Log, TEXT("Spawning CursorFX at %s"), *CursorLocation.ToString());

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CursorClickFX,
		CursorLocation,
		FRotator::ZeroRotator
	);
}
