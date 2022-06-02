// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "FlyingEnemyController.generated.h"

/**
 *
 */
UCLASS()
class REVISIONGAME4_API AFlyingEnemyController : public AAIController
{
	GENERATED_BODY()


		//Behavior Tree component 
		UBehaviorTreeComponent* BehaviourComp;

	//Blackboard component
	UBlackboardComponent* BlackBoardComp;

	//Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = AI)
		FName TargetLocationKey;

	UPROPERTY(EditDefaultsOnly, Category = AI)
		FName PlayerKey;

	TArray<AActor*> PatrolPoints;

	virtual void OnPossess(APawn* p) override;



public:
	AFlyingEnemyController();

	void SetPlayerCaught(APawn* p);

	int32 CurrentPatrolPoint = 0;
	//Inline getter functions
	FORCEINLINE UBlackboardComponent* GetBlackBoardComp() const { return BlackBoardComp; }
	FORCEINLINE TArray<AActor*> GetPatrolPoints() const { return PatrolPoints; }
};
