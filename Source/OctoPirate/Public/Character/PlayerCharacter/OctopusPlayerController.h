
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "NiagaraSystem.h"
#include "NavigationSystem.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> CursorClickFX;

protected:
	virtual void SetupInputComponent() override;
	virtual void PlayerTick( float DeltaTime ) override;
	virtual void BeginPlay() override;
	
private:
	void OnRightMousePressed();
	void OnRightMouseReleased();
	void MoveToCursor() const;
	void SpawnCursorFX();
	
	bool bToggle = false;
};
