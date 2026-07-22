#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "NiagaraSystem.h"
#include "SuicideCrab.generated.h"

UCLASS()
class OCTOPIRATE_API ASuicideCrab : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	ASuicideCrab();

protected:
	virtual void BeginPlay() override;
	
    virtual void PerformAttack_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideCrab")
	float ExplosionTriggerRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideCrab")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideCrab")
	float ExplosionDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideCrab")
	TObjectPtr<UAnimMontage> ExplosionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideCrab|VFX")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	virtual void OnDeath_Implementation() override;

private:
	bool bIsExploding = false;

	void TriggerExplosion();
	void Explode();

	UFUNCTION()
	void OnExplosionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void Tick(float DeltaTime) override;
};