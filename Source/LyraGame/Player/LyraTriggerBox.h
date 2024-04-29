// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Actor.h"
#include "LyraTriggerBox.generated.h"

UCLASS()
class LYRAGAME_API ALyraTriggerBox : public AActor
{
	GENERATED_BODY()

public:
	ALyraTriggerBox();

private:
#pragma region Delegate function
  	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UBoxComponent* OverlapBoxComponent{nullptr};

	UPROPERTY()
	FActiveGameplayEffectHandle EffectHandle;
};
