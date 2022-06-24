// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIPatrolPoint.h"
#include "FlyingEnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SteerToTarget.generated.h"


/**
 *
 */
UCLASS()
class REVISIONGAME4_API USteerToTarget : public UBTTaskNode
{
	GENERATED_BODY()
		virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void Steer(FVector target);
	void FlapWings(FVector target);
	UPROPERTY(EditAnywhere)
	float flapSpeedMax = 1.0f;
	float flapSpeed = flapSpeedMax;
	UPROPERTY(EditAnywhere)
	float flapSpeedMin = 0.4f;


	float flapTimerMax = 1.0f;
	float flapTimer = flapTimerMax;
	UPROPERTY(EditAnywhere)
	float flapForce = 600000.0f;
	AFlyingEnemyController* AICon;
	bool applyDownwardSteering = false;
	UPROPERTY(EditAnywhere)
	float heightAboveTarget = 1700.0f;
};
