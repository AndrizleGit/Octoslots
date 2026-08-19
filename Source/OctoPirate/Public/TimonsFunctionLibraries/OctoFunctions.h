// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OctoFunctions.generated.h"

/**
 * 
 */
UCLASS()
class OCTOPIRATE_API UOctoFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsSceneFullyReady(const UObject* WorldContextObject);
	
};
