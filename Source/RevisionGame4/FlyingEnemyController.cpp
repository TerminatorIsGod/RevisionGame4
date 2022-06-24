// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyingEnemyController.h"
#include "FlyingEnemy.h"
#include "AIPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"

AFlyingEnemyController::AFlyingEnemyController()
{
	//Init blackboard and behvaiour tree
	BehaviourComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackBoardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	//CurrentPatrolPoint = 0;
}

void AFlyingEnemyController::SetPlayerCaught(APawn* p)
{
	if (BlackBoardComp)
	{
		BlackBoardComp->SetValueAsObject("Player", p);
	}
}

void AFlyingEnemyController::OnPossess(APawn* p)
{
	Super::OnPossess(p);

	//Get reference to character
	AFlyingEnemy* AICharacter = Cast<AFlyingEnemy>(p);

	if (AICharacter)
	{
		if (AICharacter->BehaviorTree->BlackboardAsset)
			BlackBoardComp->InitializeBlackboard(*(AICharacter->BehaviorTree->BlackboardAsset));

		//Populate patrol point array
		//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIPatrolPoint::StaticClass(), PatrolPoints);

		BehaviourComp->StartTree(*AICharacter->BehaviorTree);
	}

}


