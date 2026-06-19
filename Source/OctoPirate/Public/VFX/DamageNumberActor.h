#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;

UCLASS()
class OCTOPIRATE_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADamageNumberActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void Setup(float DamageAmount);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float FloatUpSpeed = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float LifeTime = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Number")
	TSubclassOf<UUserWidget> WidgetClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Number")
	TObjectPtr<UWidgetComponent> WidgetComponent;
};
