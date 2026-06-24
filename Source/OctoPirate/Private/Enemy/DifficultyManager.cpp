#include "Enemy/DifficultyManager.h"
#include "Character/PlayerCharacter/OctopusCharacter.h"
#include "Character/AttributeSets/BasicAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADifficultyManager::ADifficultyManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADifficultyManager::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AOctopusCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void ADifficultyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	
	// base coefficient from time
	float NewCoefficient = ElapsedTime * CoefficientPerSecond;
	
	if (PlayerCharacter && PlayerCharacter->InRunUpgradeManager)
	{
		const int32 PlayerLevel = PlayerCharacter->InRunUpgradeManager->GetCurrentLevel();
		NewCoefficient += PlayerLevel * CoefficientPerPlayerLevel;
	}
	
	DifficultyCoefficient = NewCoefficient;
}

ADifficultyManager* ADifficultyManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	
	for (TActorIterator<ADifficultyManager> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}