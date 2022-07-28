// Fill out your copyright notice in the Description page of Project Settings.


#include "BTSelectPatrolPoint.h"
#include "AIPatrolPoint.h"
#include "FlyingEnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTSelectPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFlyingEnemyController* AICon = Cast<AFlyingEnemyController>(OwnerComp.GetAIOwner());

	if (AICon)
	{

		//Get BB component
		UBlackboardComponent* BlackBoardComp = AICon->GetBlackBoardComp();

		AAIPatrolPoint* CurrentPoint = Cast<AAIPatrolPoint>(BlackBoardComp->GetValueAsObject("TargetLocation"));

		TArray<AActor*> AvailablePatrolPoints = AICon->GetPatrolPoints();

		

		AAIPatrolPoint* NextPatrolPoint = nullptr;

		if (AICon->CurrentPatrolPoint != AvailablePatrolPoints.Num() - 1)
		{
			NextPatrolPoint = Cast<AAIPatrolPoint>(AvailablePatrolPoints[++AICon->CurrentPatrolPoint]);
		}
		else
		{
			NextPatrolPoint = Cast<AAIPatrolPoint>(AvailablePatrolPoints[0]);
			AICon->CurrentPatrolPoint = 0;
		}

		BlackBoardComp->SetValueAsObject("TargetLocation", NextPatrolPoint);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
