// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "Creature.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PacifyTriggerVolume.generated.h"

/**
 * 
 */
UCLASS()
class REVISIONGAME4_API APacifyTriggerVolume : public ATriggerVolume
{
	GENERATED_BODY()
	
public:
	APacifyTriggerVolume();
	void activateTrigger(bool isActive);

	UPROPERTY(EditAnywhere)
		float launchForce = 6000.0f;
protected:
	virtual void BeginPlay() override;
	bool activated = true;


private:
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
};
