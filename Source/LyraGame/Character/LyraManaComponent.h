// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "LyraManaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLyraMana_AttributeChanged, ULyraManaComponent*, ManaComponent, float, OldValue, float, NewValue, AActor*, Instigator);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraManaComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraManaComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the Mana component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Mana")
	static ULyraManaComponent* FindManaComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraManaComponent>() : nullptr); }

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Mana")
	void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Mana")
	void UninitializeFromAbilitySystem();

	// Returns the current Mana value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Mana")
	float GetMana() const;

	// Returns the current maximum Mana value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Mana")
	float GetMaxMana() const;

	// Returns the current Mana in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Lyra|Mana")
	float GetManaNormalized() const;

	// Delegate fired when the Mana value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FLyraMana_AttributeChanged OnManaChanged;

	// Delegate fired when the max Mana value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FLyraMana_AttributeChanged OnMaxManaChanged;

protected:
	virtual void OnUnregister() override;

	void ClearGameplayTags() const;
	
	virtual void HandleManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfMana(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;

	// Mana set used by this component.
	UPROPERTY()
	TObjectPtr<const class ULyraManaSet> ManaSet;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = GameplayEffect)
	TSubclassOf<UGameplayEffect> RecoverGameplayEffect;

	UPROPERTY()
	FActiveGameplayEffectHandle EffectHandle;

	FTimerHandle RecoverTimerHandle;
};
