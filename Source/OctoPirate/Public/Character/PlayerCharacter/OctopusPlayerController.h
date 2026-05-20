
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "OctopusPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OCTOPIRATE_API AOctopusPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AOctopusPlayerController();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> OctopusMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RightClickAction;
	
protected:
	virtual void SetupInputComponent() override;
	virtual void PlayerTick( float DeltaTime ) override;
	virtual void BeginPlay() override;
	
private:
	void OnRightMousePressed();
	void OnRightMouseReleased();
	void MoveToCursor() const;
	
	bool bRightMouseHeld = false;
};
