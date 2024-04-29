// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraTriggerBox.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Character/LyraHeroComponent.h"
#include "Components/BoxComponent.h"
#include "Messages/LyraVerbMessage.h"

// Sets default values
ALyraTriggerBox::ALyraTriggerBox()
{
	PrimaryActorTick.bCanEverTick = false;

	OverlapBoxComponent = CreateDefaultSubobject<UBoxComponent>("Box collision component");
	OverlapBoxComponent->SetBoxExtent(FVector{ 100.0 });
	
	OverlapBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::BeginOverlap);
	OverlapBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::EndOverlap);

	SetRootComponent(OverlapBoxComponent);
}

void ALyraTriggerBox::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const ALyraCharacter* OverlapCharacter{ Cast<ALyraCharacter>(OtherActor) };
	if (OverlapCharacter == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("LyraCharacter is nullptr"));
#endif
		return;
	}
	
	const ULyraHeroComponent* LyraHeroComponent{ULyraHeroComponent::FindHeroComponent(OtherActor)};
	if (LyraHeroComponent == nullptr) return;
	
	UGameplayEffect* ManaEffectClass{LyraHeroComponent->GetManaDecreaseClass()};
	if (ManaEffectClass == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ManaEffectClass is not exposed"));
#endif
	}

	ULyraAbilitySystemComponent* LyraASC{ OverlapCharacter->GetLyraAbilitySystemComponent() };
	
	if (LyraASC == nullptr)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent is not exist on %s"), *OverlapCharacter->GetName());
#endif
		return;
	}
	
	EffectHandle = LyraASC->ApplyGameplayEffectToTarget(ManaEffectClass, LyraASC);
}

void ALyraTriggerBox::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (EffectHandle.IsValid())
	{
		UAbilitySystemComponent* LyraASC{ EffectHandle.GetOwningAbilitySystemComponent() };

		UGameplayEffect* ManaCooldownClass{ULyraHeroComponent::FindHeroComponent(OtherActor)->GetManaCooldownClass()};
		
		if (ManaCooldownClass == nullptr)
		{
#if WITH_EDITOR
			UE_LOG(LogTemp, Warning, TEXT("ManaCooldownClass is not exposed"));
#endif
		}
		
		LyraASC->RemoveActiveGameplayEffect(EffectHandle, 1);
		LyraASC->ApplyGameplayEffectToTarget(ManaCooldownClass, LyraASC);
		
		EffectHandle.Invalidate();
	}
}