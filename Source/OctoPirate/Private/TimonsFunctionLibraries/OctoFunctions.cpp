// Fill out your copyright notice in the Description page of Project Settings.


#include "TimonsFunctionLibraries/OctoFunctions.h"

#include "AssetCompilingManager.h"
#include "ShaderCompiler.h"

bool UOctoFunctions::IsSceneFullyReady(const UObject* WorldContextObject)
{
	const bool bAssetsLoading = FAssetCompilingManager::Get().GetNumRemainingAssets() > 0;
	const bool bShaderCompiling = GShaderCompilingManager && GShaderCompilingManager->IsCompiling();
	if (bAssetsLoading || bShaderCompiling || IsAsyncLoading()) return false;

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		for (const ULevelStreaming* Level : World->GetStreamingLevels())
		{
			if (Level && Level->HasLoadRequestPending())
			{
				return false;
			}
		}
		if (World->IsVisibilityRequestPending())
		{
			return false;
		}
	}

	return true;
}