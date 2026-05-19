#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AOctopusCharacter::AOctopusCharacter()
{
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
}

void AOctopusCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AOctopusCharacter::SetMoveDestination(const FVector& Destination)
{
	AController* MyController = GetController();
	if (!MyController) return;
	
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, Destination);
}

