// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraManaComponent.h"

#include "LyraLogChannels.h"
#include "LyraGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraManaSet.h"
#include "Messages/LyraVerbMessage.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraManaComponent)

ULyraManaComponent::ULyraManaComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	ManaSet = nullptr;
}

void ULyraManaComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Mana component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize Mana component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	ManaSet = AbilitySystemComponent->GetSet<ULyraManaSet>();
	if (!ManaSet)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize Mana component for owner [%s] with NULL Mana set on the ability system."), *GetNameSafe(Owner));
		return;
	}
	
	// Register to listen for attribute changes.
	ManaSet->OnManaChanged.AddUObject(this, &ThisClass::HandleManaChanged);
	ManaSet->OnMaxManaChanged.AddUObject(this, &ThisClass::HandleMaxManaChanged);
	ManaSet->OnOutOfMana.AddUObject(this, &ThisClass::HandleOutOfMana);

	// TEMP: Reset attributes to default values.  Eventually this will be driven by a spread sheet.
	AbilitySystemComponent->SetNumericAttributeBase(ULyraManaSet::GetManaAttribute(), ManaSet->GetMaxMana());

	ClearGameplayTags();
	
	OnManaChanged.Broadcast(this, ManaSet->GetMana(), ManaSet->GetMana(), nullptr);
	OnMaxManaChanged.Broadcast(this, ManaSet->GetMana(), ManaSet->GetMana(), nullptr);
}

void ULyraManaComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();
	
	if (ManaSet)
	{
		ManaSet->OnManaChanged.RemoveAll(this);
		ManaSet->OnMaxManaChanged.RemoveAll(this);
		ManaSet->OnOutOfMana.RemoveAll(this);
	}

	ManaSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float ULyraManaComponent::GetMana() const
{
	return (ManaSet ? ManaSet->GetMana() : 0.0f);
}

float ULyraManaComponent::GetMaxMana() const
{
	return (ManaSet ? ManaSet->GetMaxMana() : 0.0f);
}

float ULyraManaComponent::GetManaNormalized() const
{
	if (ManaSet)
	{
		const float Mana = ManaSet->GetMana();
		const float MaxMana = ManaSet->GetMaxMana();

		return ((MaxMana > 0.0f) ? (Mana / MaxMana) : 0.0f);
	}

	return 0.0f;
}

void ULyraManaComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	
	Super::OnUnregister();
}

void ULyraManaComponent::ClearGameplayTags() const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(LyraGameplayTags::Ability_ManaShield, 0);
	}
}

void ULyraManaComponent::HandleManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULyraManaComponent::HandleMaxManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULyraManaComponent::HandleOutOfMana(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	if (AbilitySystemComponent)
	{
		const FGameplayTagContainer GameplayTagContainer{LyraGameplayTags::Ability_ManaShield};
		AbilitySystemComponent->CancelAbilities(&GameplayTagContainer);
	}
}