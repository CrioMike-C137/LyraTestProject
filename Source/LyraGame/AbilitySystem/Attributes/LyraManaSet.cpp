// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraManaSet.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "LyraGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraManaSet)

ULyraManaSet::ULyraManaSet()
	: Mana(100.0f)
	, MaxMana(100.0f)
{
	bOutOfMana = false;
	MaxManaBeforeAttributeChange = 0.0f;
	ManaBeforeAttributeChange = 0.0f;
}

void ULyraManaSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ULyraManaSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULyraManaSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void ULyraManaSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraManaSet, Mana, OldValue);

	const float CurrentMana = GetMana();
	const float EstimatedMagnitude = CurrentMana - OldValue.GetCurrentValue();
	
	OnManaChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMana);

	if (!bOutOfMana && CurrentMana <= 0.0f)
	{
		OnOutOfMana.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMana);
	}
	
	bOutOfMana = (CurrentMana <= 0.0f);
}

void ULyraManaSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraManaSet, MaxMana, OldValue);

	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	OnMaxManaChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxMana() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxMana());
}

bool ULyraManaSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}
	
	// Save the current Mana
	ManaBeforeAttributeChange = GetMana();
	MaxManaBeforeAttributeChange = GetMaxMana();

	return true;
}

void ULyraManaSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetManaDecreaseAttribute())
	{
		// Convert into -Mana and then clamp
		SetMana(FMath::Clamp(GetMana() - GetManaDecrease(), 0.f, GetMaxMana()));
		SetManaDecrease(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetManaRecoverAttribute())
	{
		// Convert into +Mana and then clamp
		SetMana(FMath::Clamp(GetMana() + GetManaRecover(), 0.f, GetMaxMana()));
		SetManaRecover(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Clamp and fall into out of Mana handling below
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		// Notify on any requested max Mana changes
		OnMaxManaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxManaBeforeAttributeChange, GetMaxMana());
	}

	// If Mana has actually changed activate callbacks
	if (GetMana() != ManaBeforeAttributeChange)
	{
		OnManaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, ManaBeforeAttributeChange, GetMana());
	}

	if ((GetMana() <= 0.0f) && !bOutOfMana)
	{
		OnOutOfMana.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, ManaBeforeAttributeChange, GetMana());
	}

	// Check Mana again in case an event above changed it.
	bOutOfMana = (GetMana() <= 0.0f);
}

void ULyraManaSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void ULyraManaSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void ULyraManaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxManaAttribute())
	{
		// Make sure current Mana is not greater than the new max Mana.
		if (GetMana() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetManaAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfMana && (GetMana() > 0.0f))
	{
		bOutOfMana = false;
	}
}

void ULyraManaSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetManaAttribute())
	{
		// Do not allow Mana to go negative or above max Mana.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		// Do not allow max Mana to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
