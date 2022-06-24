// Fill out your copyright notice in the Description page of Project Settings.


#include "SteerToTarget.h"


EBTNodeResult::Type USteerToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AFlyingEnemyController>(OwnerComp.GetAIOwner());

	if (AICon)
	{
		UBlackboardComponent* BlackBoardComp = AICon->GetBlackBoardComp();
		//AAIPatrolPoint* CurrentPoint = Cast<AAIPatrolPoint>(BlackBoardComp->GetValueAsObject("TargetLocation"));

		APawn* CurrentPoint = Cast<APawn>(BlackBoardComp->GetValueAsObject("Player"));


		if (CurrentPoint != nullptr)
		{
			BlackBoardComp->SetValueAsVector("TargetLocation", CurrentPoint->GetActorLocation());
			Steer(BlackBoardComp->GetValueAsVector("TargetLocation"));
			FlapWings(BlackBoardComp->GetValueAsVector("TargetLocation"));

			return EBTNodeResult::Succeeded;
		}
		return EBTNodeResult::Failed;

	}

	return EBTNodeResult::Failed;

}

void USteerToTarget::Steer(FVector target)
{
	//I don't really like how this steering makes the bird end up directly atop the player.
	//Modifying this behaviour so it kinda orbits or wanders around the player would be better.
	ACharacter* aiChar = Cast<ACharacter>(AICon->GetPawn());
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();


	FVector newVel = aiChar->GetCharacterMovement()->Velocity;

	//Desired Velocity
	FVector desiredVel = target - aiChar->GetCharacterMovement()->GetActorLocation();
	desiredVel *= 2.5f;

	//Steering Force
	FVector steering = desiredVel - aiChar->GetCharacterMovement()->Velocity;
	steering.Normalize();
	steering /= aiChar->GetCharacterMovement()->Mass;

	//Add to velocity
	newVel += steering * 25000.0f * DeltaTime * 2.5;

	//MeshRootComp->SetPhysicsLinearVelocity(newVel);
	if (!applyDownwardSteering)
		aiChar->GetCharacterMovement()->Velocity = FVector3d(newVel.X, newVel.Y, aiChar->GetCharacterMovement()->Velocity.Z);
	else
		aiChar->GetCharacterMovement()->Velocity = FVector3d(newVel.X, newVel.Y, newVel.Z / 1.5f );



	aiChar->SetActorRotation(aiChar->GetCharacterMovement()->Velocity.ToOrientationQuat());
}

void USteerToTarget::FlapWings(FVector target)  
{
	ACharacter* aiChar = Cast<ACharacter>(AICon->GetPawn());
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds(); 

	if (aiChar->GetCharacterMovement()->GetActorLocation().Z < target.Z + heightAboveTarget)
	{
		flapSpeed = (target.Z + heightAboveTarget - aiChar->GetCharacterMovement()->GetActorLocation().Z) / 150.0f;
		applyDownwardSteering = false;
	}
	else 
	{
		
		flapSpeed = FMath::Lerp(flapSpeed, flapSpeedMin, DeltaTime);

		//Dive? Maybe this should only happen if something is really far down and straight below the bird. 
		applyDownwardSteering = true;

	}

	flapTimer -= DeltaTime * flapSpeed;

	if (flapTimer <= 0.0f)
	{
		flapTimer = flapTimerMax;
		aiChar->GetCharacterMovement()->AddForce(FVector3d(0.0f, 0.0f, flapForce));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("--WINGS FLAPPED--"));

	}
}
