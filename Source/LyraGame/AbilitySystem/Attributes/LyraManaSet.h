// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "LyraAttributeSet.h"
#include "NativeGameplayTags.h"

#include "LyraManaSet.generated.h"

class UObject;
struct FFrame;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Lyra_Damage_Message);

struct FGameplayEffectModCallbackData;

/**
 * ULyraManaSet
 *
 *	Class that defines attributes for use special ability (magic)
 */

UCLASS(BlueprintType)
class LYRAGAME_API ULyraManaSet : public ULyraAttributeSet
{
	GENERATED_BODY()

public:
	ULyraManaSet();

	ATTRIBUTE_ACCESSORS(ULyraManaSet, Mana);
	ATTRIBUTE_ACCESSORS(ULyraManaSet, MaxMana);
	ATTRIBUTE_ACCESSORS(ULyraManaSet, ManaRecover);
	ATTRIBUTE_ACCESSORS(ULyraManaSet, ManaDecrease);

	// Delegate when mana changes due to damage/healing, some information may be missing on the client
	mutable FLyraAttributeEvent OnManaChanged;

	// Delegate when max mana changes
	mutable FLyraAttributeEvent OnMaxManaChanged;

	// Delegate to broadcast when the mana attribute reaches zero
	mutable FLyraAttributeEvent OnOutOfMana;

protected:
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Lyra|Mana", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Mana;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Lyra|Mana", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana;

	// Used to track when the health reaches 0.
	bool bOutOfMana;

	// Store the health before any changes 
	float MaxManaBeforeAttributeChange;
	float ManaBeforeAttributeChange;

	// -------------------------------------------------------------------
	//	Meta Attribute
	// -------------------------------------------------------------------

	// Incoming mana. This is mapped directly to +Mana (infinite effect)
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Mana", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData ManaRecover;

	// Decrease mana. This is mapped directly to -Mana (temporary/infinite effect)
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Mana", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData ManaDecrease;
};
