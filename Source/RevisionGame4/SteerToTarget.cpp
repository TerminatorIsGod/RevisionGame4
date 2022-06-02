// Fill out your copyright notice in the Description page of Project Settings.


#include "SteerToTarget.h"


EBTNodeResult::Type USteerToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AFlyingEnemyController>(OwnerComp.GetAIOwner());

	if (AICon)
	{
		UBlackboardComponent* BlackBoardComp = AICon->GetBlackBoardComp();
		AAIPatrolPoint* CurrentPoint = Cast<AAIPatrolPoint>(BlackBoardComp->GetValueAsObject("TargetLocation"));

		Steer(CurrentPoint->GetActorLocation());
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;

}

void USteerToTarget::Steer(FVector target)
{
	ACharacter* aiChar = Cast<ACharacter>(AICon->GetPawn());
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();

	FVector newVel = aiChar->GetCharacterMovement()->Velocity;

	//Desired Velocity
	FVector desiredVel = target - aiChar->GetCharacterMovement()->GetActorLocation();
	desiredVel *= 4.5f;

	//Steering Force
	FVector steering = desiredVel - aiChar->GetCharacterMovement()->Velocity;
	steering.Normalize();
	steering /= aiChar->GetCharacterMovement()->Mass;

	//Add to velocity
	newVel += steering * 45000.0f * DeltaTime * 4.5;

	//MeshRootComp->SetPhysicsLinearVelocity(newVel);
	aiChar->GetCharacterMovement()->Velocity = newVel;

}