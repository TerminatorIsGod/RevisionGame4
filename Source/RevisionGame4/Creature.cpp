// Fill out your copyright notice in the Description page of Project Settings.


#include "Creature.h"

// Sets default values
ACreature::ACreature()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACreature::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACreature::Select(float DeltaTime)
{
}

// Called every frame
void ACreature::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BecomePacified(DeltaTime);
}

// Called to bind functionality to input
void ACreature::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACreature::Pull(float DeltaTime, int i, FVector target)
{

	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(pulledActors[i]->GetRootComponent());
	FVector newVel = MeshRootComp->GetPhysicsLinearVelocity();

	//Desired Velocity
	FVector desiredVel = target - MeshRootComp->GetComponentLocation();
	desiredVel *= maxTKPullSpeed;

	//Steering Force
	FVector steering = desiredVel - MeshRootComp->GetPhysicsLinearVelocity();
	steering.Normalize();
	steering /= MeshRootComp->GetMass() * 0.05f;

	//Add to velocity
	newVel += steering * 45000.0f * DeltaTime * maxTKPullSpeed;
	//MeshRootComp->AddForce(newVel);
	//if (newVel.Size() > )
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Running: %s"), ("AHH"));
	//	newVel.Normalize();
	//	newVel *= maxTKPullSpeed;
	//}

	MeshRootComp->SetPhysicsLinearVelocity(newVel);

}

void ACreature::Catch(float DeltaTime, int i, FVector target)
{
	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(pulledActors[i]->GetRootComponent());
	FVector dir = target - pulledActors[i]->GetActorLocation();
	float dist = dir.Size();

	if (dist < catchRadius && MeshRootComp->GetMass() + massTotal <= massLimit)
	{
		if (!caughtActors.Contains(pulledActors[i]))
		{
			caughtActors.Add(pulledActors[i]);
			caughtActorsToRemove.Add(pulledActors[i]);
			massTotal += MeshRootComp->GetMass();
		}
	}
}

void ACreature::Throw(float DeltaTime)
{
	if (throwCharging && caughtActors.Num() != 0)
	{
		if (TKCharge < TKChargeMax)
		{
			UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[caughtActors.Num() - 1]->GetRootComponent());
			TKCharge += (TKChargeRate / (MeshRootComp->GetMass() * 0.25)) * DeltaTime;
		}

		if (throwCount > caughtActors.Num())
			throwCount = caughtActors.Num();

		if (throwCount < 1)
			throwCount = 1;

		for (int i = caughtActors.Num() - 1; i >= (caughtActors.Num() - throwCount); i--)
		{
			//Steering Stuff
			UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[i]->GetRootComponent());
			FVector newVel = MeshRootComp->GetPhysicsLinearVelocity();

			//Desired Velocity 
			FVector desiredVel;

			if (caughtActors[i]->GetOwner()->ActorHasTag("Platform"))
			{
				desiredVel = GetActorLocation() - MeshRootComp->GetComponentLocation();
			}
			else
			{
				desiredVel = frontTarget->GetComponentLocation() - MeshRootComp->GetComponentLocation();
			}
			desiredVel *= maxTKPullSpeed * 2.0f;

			//Steering Force
			FVector steering = desiredVel - MeshRootComp->GetPhysicsLinearVelocity();
			steering.Normalize();
			steering /= MeshRootComp->GetMass() * 0.05f;

			//Add to velocity
			newVel += steering * 45000.0f * DeltaTime * maxTKPullSpeed * 2.0f;

			//Set Velocity
			MeshRootComp->SetPhysicsLinearVelocity(newVel);
		}
	}

	//if (!holdingLeftClick && throwCount > 0)
	if (!throwCharging && throwCount > 0)
	{

		// Set up parameters for getting the player viewport
		FVector PlayerViewPointLocation;
		FRotator PlayerViewPointRotation;

		// Get player viewport and set these parameters
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
			OUT PlayerViewPointLocation,
			OUT PlayerViewPointRotation
		);

		// Parameter for how far out the the line trace reaches
		float Reach = 100000.f;
		LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;

		// Set parameters to use line tracing
		FHitResult Hit;
		FCollisionQueryParams TraceParams(FName(TEXT("")), true, GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

		// Raycast out to this distance
		GetWorld()->LineTraceSingleByObjectType(
			OUT Hit,
			PlayerViewPointLocation,
			LineTraceEnd,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic), //Makes it so ray only intercects with static geometry. Might need to make this intercect with anything so it aims at enemies properly.
			TraceParams
		);

		for (int i = caughtActors.Num() - 1; i >= (caughtActors.Num() - throwCount); i--)
		{
			FVector dir;
			UStaticMeshComponent* actorToThrow = Cast<UStaticMeshComponent>(caughtActors[i]->GetRootComponent());

			if (Hit.ImpactPoint != FVector3d(0.0f))
				dir = Hit.ImpactPoint - actorToThrow->GetComponentLocation();
			else
				dir = LineTraceEnd - actorToThrow->GetComponentLocation();

			dir.Normalize();
			actorToThrow->SetPhysicsLinearVelocity(dir * TKCharge);
			massTotal -= actorToThrow->GetMass();
		}

		for (int i = 0; i < throwCount; i++)
		{
			caughtActors.Pop();
		}

		throwCount = 0;
		TKCharge = 0;
	}
}

void ACreature::CatchingPulling(float DeltaTime)
{
	//Objects being pulled
	for (int p = 0; p < pulledActors.Num(); p++)
	{
		if (pulledActors[p]->ActorHasTag("Platform"))
		{
			Catch(DeltaTime, p, GetActorLocation());
			Pull(DeltaTime, p, GetActorLocation());
		}
		else
		{
			Catch(DeltaTime, p, backTarget->GetComponentLocation());
			Pull(DeltaTime, p, backTarget->GetComponentLocation());
		}
	}

	//Objects added to caught list, that need to be removed from pulled list
	for (int r = 0; r < caughtActorsToRemove.Num(); r++)
	{
		pulledActors.Remove(caughtActorsToRemove[r]);
	}
	caughtActorsToRemove.Empty();

	//Objects that Have been caught
	for (int c = 0; c < caughtActors.Num() - throwCount; c++)
	{
		if (caughtActors[c]->ActorHasTag("Platform"))
		{
			Follow(DeltaTime, c, GetActorLocation());
		}
		else
		{
			Follow(DeltaTime, c, backTarget->GetComponentLocation());
		}
	}
}

void ACreature::BecomePacified(float DeltaTime)
{
	if (isPacified && becomePacifiedTimer > 0.0f)
	{
		becomePacifiedTimer -= DeltaTime;
	}
	else
	{
		becomePacifiedTimer = becomePacifiedTimerMax;
		isPacified = false;
	}
}

void ACreature::Follow(float DeltaTime, int i, FVector target)
{
	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[i]->GetRootComponent());
	FVector newVel = MeshRootComp->GetPhysicsLinearVelocity();

	//Desired Velocity
	FVector desiredVel = target - MeshRootComp->GetComponentLocation();
	desiredVel *= maxTKPullSpeed;

	//Steering Force
	FVector steering = desiredVel - MeshRootComp->GetPhysicsLinearVelocity();
	steering.Normalize();
	steering /= MeshRootComp->GetMass() * 0.05f;

	//Add to velocity
	newVel += steering * 45000.0f * DeltaTime * maxTKPullSpeed;

	//Set Velocity
	MeshRootComp->SetPhysicsLinearVelocity(newVel);

}

