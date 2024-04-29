// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_ManaShield.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/LyraHeroComponent.h"
#include "Character/LyraManaComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraVerbMessage.h"


void ULyraGameplayAbility_ManaShield::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool ULyraGameplayAbility_ManaShield::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void ULyraGameplayAbility_ManaShield::SendMessage(const FGameplayTag& InGameplayTag) const
{
	FLyraVerbMessage Message;
	Message.Verb = InGameplayTag;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}

void ULyraGameplayAbility_ManaShield::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UGameplayEffect* ManaEffectClass{ULyraHeroComponent::FindHeroComponent(ActorInfo->AvatarActor.Get())->GetManaDecreaseClass()};
	
	if (ManaEffectClass == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ManaEffectClass is not exposed"));
#endif
	}
	
	const TWeakObjectPtr<UAbilitySystemComponent> ASC{ActorInfo->AbilitySystemComponent};
	
	if (!ASC.IsValid())
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent is nullptr in mana shield ability"));
#endif
	}
	
	EffectHandle = ASC->ApplyGameplayEffectToTarget(ManaEffectClass, ASC.Get());
	
	SendMessage(LyraGameplayTags::HUD_Slot_ManaShield_Show);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void ULyraGameplayAbility_ManaShield::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!EffectHandle.IsValid())
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("EffectHandles is empty"));
#endif
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	UAbilitySystemComponent* ASC{ EffectHandle.GetOwningAbilitySystemComponent() };
	
	if (ASC == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent is null in mana shield ability"));
#endif
		return;
	}

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ActorInfo or AvatarActor is not valid"));
#endif
		return;
	}

	UGameplayEffect* ManaCooldownClass{ULyraHeroComponent::FindHeroComponent(ActorInfo->AvatarActor.Get())->GetManaCooldownClass()};
	
	if (ManaCooldownClass == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ManaCooldownClass is not exposed"));
#endif
	}
	
	ASC->RemoveActiveGameplayEffect(EffectHandle, 1);
	ASC->ApplyGameplayEffectToTarget(ManaCooldownClass, ASC);
	
	EffectHandle.Invalidate();

	SendMessage(LyraGameplayTags::HUD_Slot_ManaShield_Hide);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
