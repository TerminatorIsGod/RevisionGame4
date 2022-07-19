// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
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
class REVISIONGAME4_API USteerToTarget : public UBTTask_BlueprintBase
{
	GENERATED_BODY() 
		virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void Steer(FVector target);
	void Orbit(FVector target);
	void Avoidance(FVector dir);
	void AvoidanceReflect(FVector dir);
	void FlapWings(FVector target);

	FVector3d MinVec3DWrap(FVector3d start, FVector3d end, float spaceLengthX, float spaceLengthY, float spaceLengthZ);

	FName GetSelectedTargetLocationKey() const;

	UPROPERTY(EditAnywhere, Category = Blackboard)
		struct FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Reach Quotas")
	float reachedDist;

	UPROPERTY(EditAnywhere, Category = "Reach Quotas")
	float reachedTime;

	float reachedTimer = 0.0f;

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
	ACharacter* aiChar;

	bool applyDownwardSteering = false;
	UPROPERTY(EditAnywhere)
	float heightAboveTarget = 1700.0f;
	UPROPERTY(EditAnywhere)
	float avoidanceRange = 1000.0f;
	UPROPERTY(EditAnywhere)
	float avoidanceStrength = 100.0f;

	UPROPERTY(EditAnywhere)
		float orbitRadius = 300.0f;

	UPROPERTY(EditAnywhere)
		float maxHeight = 4000.0f;

	UPROPERTY(EditAnywhere)
		float maxSpeed = 40.0f;

	UPROPERTY(EditAnywhere)
		float orbitCorrectionForce = 100.0f;


	//Space Wrapping related values. Probably have triggers each area that sets these values
	UPROPERTY(EditAnywhere)
		bool isSpaceWrappingX = false;

	UPROPERTY(EditAnywhere)
		bool isSpaceWrappingY = false;

	UPROPERTY(EditAnywhere)
		bool isSpaceWrappingZ = false;

	UPROPERTY(EditAnywhere)
		float wrappedSpaceLengthX = 0.0f;

	UPROPERTY(EditAnywhere)
		float wrappedSpaceLengthY = 0.0f;

	UPROPERTY(EditAnywhere)
		float wrappedSpaceLengthZ = 0.0f;

};
