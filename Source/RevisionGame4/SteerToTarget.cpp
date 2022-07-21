// Fill out your copyright notice in the Description page of Project Settings.


#include "SteerToTarget.h"


EBTNodeResult::Type USteerToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AFlyingEnemyController>(OwnerComp.GetAIOwner());

	if (AICon)
	{
		UBlackboardComponent* BlackBoardComp = AICon->GetBlackBoardComp();
		//AAIPatrolPoint* CurrentPoint = Cast<AAIPatrolPoint>(BlackBoardComp->GetValueAsObject("TargetLocation"));

		AActor* CurrentPoint = Cast<AActor>(BlackBoardComp->GetValueAsObject(TargetLocationKey.SelectedKeyName));
		aiChar = Cast<ACharacter>(AICon->GetPawn());

		
		if (CurrentPoint != nullptr)
		{
			//BlackBoardComp->SetValueAsVector("TargetLocation", CurrentPoint->GetActorLocation());

			Steer(CurrentPoint->GetActorLocation());
			FlapWings(CurrentPoint->GetActorLocation());
			AvoidanceReflect(aiChar->GetActorForwardVector());
			//Avoidance(-aiChar->GetActorUpVector());
			//AvoidanceReflect(aiChar->GetActorUpVector());


			FVector diagVec = aiChar->GetActorRightVector() + aiChar->GetActorForwardVector();
			diagVec.Normalize();

			AvoidanceReflect(diagVec);

			diagVec = -aiChar->GetActorRightVector() + aiChar->GetActorForwardVector();
			diagVec.Normalize();

			AvoidanceReflect(diagVec);

			diagVec = aiChar->GetActorUpVector() + aiChar->GetActorForwardVector();
			diagVec.Normalize();

			AvoidanceReflect(diagVec);

			diagVec = -aiChar->GetActorUpVector() + aiChar->GetActorForwardVector();
			diagVec.Normalize();

			AvoidanceReflect(diagVec);




			if (aiChar->GetCharacterMovement()->Velocity.Length() > maxSpeed)
			{
				aiChar->GetCharacterMovement()->Velocity.Normalize();
				aiChar->GetCharacterMovement()->Velocity = aiChar->GetCharacterMovement()->Velocity * maxSpeed;
			}

			aiChar->SetActorRotation(aiChar->GetCharacterMovement()->Velocity.ToOrientationQuat());

			if ((CurrentPoint->GetActorLocation() - aiChar->GetActorLocation()).Length() < reachedDist)
			{
				float DeltaTime = aiChar->GetWorld()->GetDeltaSeconds();
				reachedTimer += DeltaTime;

				FString TheFloatStr = FString::SanitizeFloat(reachedTimer);
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, *TheFloatStr);


				if (reachedTimer >= reachedTime)
				{
					reachedTimer = 0.0f;
					return EBTNodeResult::Succeeded;
				}
			}
		}
		else
		{
			return EBTNodeResult::Succeeded;
		}
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
	//DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), LineTraceEnd, FColor::Red, false,1.0 * DeltaTime,0.0f,10.0f);

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
	//DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), LineTraceEnd, FColor::Red, false, 1.0 * DeltaTime, 0.0f, 10.0f);

	if (!ActorHit)
		return;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("AVOIDING"));

	FVector reflectedDir = dir.MirrorByVector(Hit.ImpactNormal);
	//Add avoidance force
	//FVector vel = aiChar->GetVelocity();
	//vel.Normalize();
	//FVector newDir = vel - dir;
	//FVector newDir = -dir;

	//DrawDebugLine(GetWorld(), aiChar->GetActorLocation(), aiChar->GetActorLocation() + reflectedDir * avoidanceStrength / 5.0f, FColor::Green, false, 1.0f, 0.0f, 15.0f);

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

FVector3d USteerToTarget::MinVec3DWrap(FVector3d start, FVector3d end, float spaceLengthX = 0.0f, float spaceLengthY = 0.0f, float spaceLengthZ = 0.0f)
{
	float x = end.X - start.X;
	float y = end.Y - start.Y;
	float z = end.Z - start.Z;

	if (spaceLengthX > 0.0f && std::abs(x) > (spaceLengthX / 2.0f))
		x = (spaceLengthX - std::abs(x)) * -(x/x); //FIGURED IT OUT!

	if (spaceLengthY > 0.0f && std::abs(y) > (spaceLengthY / 2.0f))
		y = (spaceLengthY - std::abs(y)) * -(y/y);

	if (spaceLengthZ > 0.0f && std::abs(z) > (spaceLengthZ / 2.0f))
		z = (spaceLengthZ - std::abs(z)) * -(z/z);

	return FVector3d(x, y, z);

}

FORCEINLINE FName USteerToTarget::GetSelectedTargetLocationKey() const
{
	return TargetLocationKey.SelectedKeyName;
}