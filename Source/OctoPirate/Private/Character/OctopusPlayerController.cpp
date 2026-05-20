#include "Character/OctopusPlayerController.h"
#include "Character/OctopusCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

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
}

void AOctopusPlayerController::OnRightMouseReleased()
{
	bRightMouseHeld = false;
}

void AOctopusPlayerController::MoveToCursor() const
{
	FHitResult HitResult;
	
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	
	if (!bHit) return;
	
	AOctopusCharacter* OctopusChar = Cast<AOctopusCharacter>(GetPawn());
	if (!OctopusChar) return;
	
	OctopusChar->SetMoveDestination(HitResult.ImpactPoint);
}
