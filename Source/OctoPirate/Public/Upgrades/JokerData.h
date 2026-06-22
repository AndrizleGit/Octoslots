#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JokerData.generated.h"

UCLASS()
class OCTOPIRATE_API UJokerData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joker")
	FText JokerName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joker")
	FText JokerDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joker")
	TObjectPtr<UTexture2D> JokerIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joker")
	FName JokerEffectID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joker")
	float JokerValue = 0.0f;
	
};
