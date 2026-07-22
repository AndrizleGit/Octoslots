#include "Character/PlayerCharacter/OctopusPlayerController.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"

#define TRACE_GROUND ECC_GameTraceChannel1

AOctopusPlayerController::AOctopusPlayerController()
{
	bToggle = true;
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
		
		if (LeftClickAction)
		{
			EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AOctopusPlayerController::OnLeftClickPressed);
			EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AOctopusPlayerController::OnLeftClickReleased);
		}
	}
}

void AOctopusPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	if (bToggle)
	{
		MoveToCursor();
	}
}

void AOctopusPlayerController::OnRightMousePressed()
{
	bToggle = !bToggle;
	SpawnCursorFX();
}

void AOctopusPlayerController::OnLeftClickPressed()
{
	AOctopusCharacter* OctopusChar = Cast<AOctopusCharacter>(GetPawn());
	if (!OctopusChar) return;
	OctopusChar->TriggerDeflect();
}

void AOctopusPlayerController::OnLeftClickReleased()
{
	// nothing for now
	// deflect stays active until DeflectDuration expires
}


void AOctopusPlayerController::OnRightMouseReleased()
{
	
}

void AOctopusPlayerController::MoveToCursor() const
{
	FHitResult HitResult;
	if (!GetHitResultUnderCursor(TRACE_GROUND, false, HitResult)) return;

	bool bHit = GetHitResultUnderCursor(TRACE_GROUND, false, HitResult);

	if (!bHit) return;
	
	AOctopusCharacter* OctopusChar = Cast<AOctopusCharacter>(GetPawn());
	if (!OctopusChar) return;

	FVector Destination = HitResult.ImpactPoint;

	// Skip redundant nav queries/moves if cursor hasn't moved much
	static FVector LastDestination = FVector::ZeroVector;
	if (FVector::DistSquared(Destination, LastDestination) < FMath::Square(25.f))
	{
		return;
	}
	// Find nearest available Navmesh
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation ProjectedLocation;
		const FVector QueryExtent(500.f, 500.f, 700.f);

		if (NavSys->ProjectPointToNavigation(Destination, ProjectedLocation, QueryExtent))
		{
			Destination = ProjectedLocation.Location;
		}
		else
		{
			return;
		}
	}

	LastDestination = Destination;
	OctopusChar->SetMoveDestination(Destination);
}

void AOctopusPlayerController::SpawnCursorFX()
{
	if (!CursorClickFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursorClickFX is not assigned!"));
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(TRACE_GROUND, false, HitResult))
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