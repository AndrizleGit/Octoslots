// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoShootAbility.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
void UAutoShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Check if the actor is valid Character reference
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		// Make the Character Shoot
		Character->Jump();

	}
	// End the ability 
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}