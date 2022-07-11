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
		aiChar = Cast<ACharacter>(AICon->GetPawn());


		if (CurrentPoint != nullptr)
		{
			BlackBoardComp->SetValueAsVector("TargetLocation", CurrentPoint->GetActorLocation());

			Steer(BlackBoardComp->GetValueAsVector("TargetLocation"));
			FlapWings(BlackBoardComp->GetValueAsVector("TargetLocation"));
			AvoidanceReflect(aiChar->GetActorForwardVector());
			//Avoidance(aiChar->GetActorRightVector() / 1.2f);
			//Avoidance(-aiChar->GetActorRightVector() / 1.2f);
			Avoidance(-aiChar->GetActorUpVector());

			FVector diagVec = aiChar->GetActorRightVector() + aiChar->GetActorForwardVector();
			diagVec.Normalize();

			FVector diagVec2 = -aiChar->GetActorRightVector() + aiChar->GetActorForwardVector();
			diagVec2.Normalize();

			AvoidanceReflect(diagVec);
			AvoidanceReflect(diagVec2);


			if (aiChar->GetCharacterMovement()->Velocity.Length() > maxSpeed)
			{
				aiChar->GetCharacterMovement()->Velocity.Normalize();
				aiChar->GetCharacterMovement()->Velocity = aiChar->GetCharacterMovement()->Velocity * maxSpeed;
			}

			aiChar->SetActorRotation(aiChar->GetCharacterMovement()->Velocity.ToOrientationQuat());

			return EBTNodeResult::Succeeded;

		}
		return EBTNodeResult::Failed;

	}

	return EBTNodeResult::Failed;

}


void USteerToTarget::Steer(FVector target)
{
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();

	FVector newVel = aiChar->GetCharacterMovement()->Velocity;

	//Desired Velocity
	FVector desiredVel = target - aiChar->GetCharacterMovement()->GetActorLocation();

	//Orbit Check
	if (desiredVel.Size() < orbitRadius)
	{
		Orbit(target);
		return;
	}

	//desiredVel *= 2.5f;

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

}

void USteerToTarget::Orbit(FVector target)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("IN ORBIT"));

	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();


	FVector2d newVel = FVector2d(aiChar->GetCharacterMovement()->Velocity.X, aiChar->GetCharacterMovement()->Velocity.Y);

	//Desired Velocity
	FVector2d desiredVel = FVector2d(target.X, target.Y) - FVector2d(aiChar->GetCharacterMovement()->GetActorLocation().X, aiChar->GetCharacterMovement()->GetActorLocation().Y);
	FVector2d perpendicularVel = FVector2d(desiredVel.Y, -desiredVel.X); //Perpendicular vector
	//perpendicularVel *= 2.0f;


	desiredVel.Normalize();
	desiredVel *= orbitCorrectionForce;
	perpendicularVel -= desiredVel;
	

	//Steering Force
	FVector2D steering = perpendicularVel - FVector2d(aiChar->GetCharacterMovement()->Velocity.X, aiChar->GetCharacterMovement()->Velocity.Y);
	steering.Normalize();
	steering /= aiChar->GetCharacterMovement()->Mass;

	//Add to velocity
	newVel += steering * 25000.0f * DeltaTime * 2.0;


	aiChar->GetCharacterMovement()->Velocity = FVector3d(newVel.X, newVel.Y, aiChar->GetCharacterMovement()->Velocity.Z);
}

void USteerToTarget::Avoidance(FVector dir)
{
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();

	// Parameter for how far out the the line trace reaches

	FVector LineTraceEnd =  aiChar->GetActorLocation() + dir * avoidanceRange;

	// Set parameters to use line tracing
	FHitResult Hit;
	FCollisionQueryParams TraceParams(FName(TEXT("")), false, aiChar->GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

	// Raycast out to this distance
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		aiChar->GetActorLocation(),
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
		TraceParams
	);

	// See what if anything has been hit and return what
	UPrimitiveComponent* ActorHit = Hit.GetComponent();
	DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), LineTraceEnd, FColor::Red, false,1.0 * DeltaTime,0.0f,10.0f);

	if (!ActorHit)
		return;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("AVOIDING"));


	//Add avoidance force
	//FVector vel = aiChar->GetVelocity();
	//vel.Normalize();
	//FVector newDir = vel - dir;
	FVector newDir = -dir;


	aiChar->GetCharacterMovement()->Velocity += newDir * avoidanceStrength * DeltaTime;
}

void USteerToTarget::AvoidanceReflect(FVector dir)
{
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();

	// Parameter for how far out the the line trace reaches

	FVector LineTraceEnd = aiChar->GetActorLocation() + dir * avoidanceRange;

	// Set parameters to use line tracing
	FHitResult Hit;
	FCollisionQueryParams TraceParams(FName(TEXT("")), false, aiChar->GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

	// Raycast out to this distance
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		aiChar->GetActorLocation(),
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
		TraceParams
	);

	// See what if anything has been hit and return what
	UPrimitiveComponent* ActorHit = Hit.GetComponent();
	DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), LineTraceEnd, FColor::Red, false, 1.0 * DeltaTime, 0.0f, 10.0f);

	if (!ActorHit)
		return;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("AVOIDING"));

	FVector reflectedDir = dir.MirrorByVector(Hit.ImpactNormal);
	//Add avoidance force
	//FVector vel = aiChar->GetVelocity();
	//vel.Normalize();
	//FVector newDir = vel - dir;
	//FVector newDir = -dir;

	DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), aiChar->GetActorLocation() + reflectedDir * avoidanceStrength / 5.0f, FColor::Green, false, 1.0f, 0.0f, 15.0f);

	aiChar->GetCharacterMovement()->Velocity += reflectedDir * avoidanceStrength * DeltaTime;
}

void USteerToTarget::FlapWings(FVector target)  
{
	float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds(); 

	if (aiChar->GetCharacterMovement()->GetActorLocation().Z < target.Z + heightAboveTarget && aiChar->GetCharacterMovement()->GetActorLocation().Z < maxHeight)
	{
		float newFlapSpeed = (target.Z + heightAboveTarget - aiChar->GetCharacterMovement()->GetActorLocation().Z) / 150.0f;
		if (newFlapSpeed < flapSpeedMax)
			flapSpeed = newFlapSpeed;
		else
			flapSpeed = flapSpeedMax;
		applyDownwardSteering = false;
	}
	else 
	{
		
		flapSpeed = FMath::Lerp(flapSpeed, flapSpeedMin, DeltaTime);

		//Dives down if out of orbit radius
		FVector vecToTarget = target - aiChar->GetCharacterMovement()->GetActorLocation();
		if (vecToTarget.Length() > orbitRadius)
			applyDownwardSteering = false;
		else
			applyDownwardSteering = true;

	}

	flapTimer -= DeltaTime * flapSpeed;

	if (flapTimer <= 0.0f)
	{
		flapTimer = flapTimerMax;
		aiChar->GetCharacterMovement()->AddForce(FVector3d(0.0f, 0.0f, flapForce));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("--WINGS FLAPPED--"));

	}
}
