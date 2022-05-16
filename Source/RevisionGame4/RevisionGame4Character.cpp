// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevisionGame4Character.h"
#include "RevisionGame4Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"


//////////////////////////////////////////////////////////////////////////
// AFirstPersonTestCharacter

ARevisionGame4Character::ARevisionGame4Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));


}

void ARevisionGame4Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	GetCharacterMovement()->AirControl = 10.0f;
	backTarget = GetCapsuleComponent()->GetChildComponent(3);
	frontTarget = GetCapsuleComponent()->GetChildComponent(2)->GetChildComponent(3);
}

void ARevisionGame4Character::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Error, TEXT("NAME: %s"), *(GetCapsuleComponent()->GetChildComponent(3)->GetName()));

	Hover(DeltaTime);
	Select(DeltaTime);

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

	Throw(DeltaTime);
	Grapple(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////// Input

void ARevisionGame4Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ARevisionGame4Character::StartLeftClick);
	PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &ARevisionGame4Character::StopLeftClick);

	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &ARevisionGame4Character::StartRightClick);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &ARevisionGame4Character::StopRightClick);

	PlayerInputComponent->BindAction("Hover", IE_Pressed, this, &ARevisionGame4Character::StartHover);
	PlayerInputComponent->BindAction("Hover", IE_Released, this, &ARevisionGame4Character::StopHover);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ARevisionGame4Character::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ARevisionGame4Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ARevisionGame4Character::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ARevisionGame4Character::LookUpAtRate);

	PlayerInputComponent->BindAxis("Scroll", this, &ARevisionGame4Character::Scroll);

}

void ARevisionGame4Character::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void ARevisionGame4Character::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ARevisionGame4Character::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void ARevisionGame4Character::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);

	}
}

void ARevisionGame4Character::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ARevisionGame4Character::StartHover()
{
	isHovering = true;
}

void ARevisionGame4Character::StopHover()
{
	isHovering = false;
}

void ARevisionGame4Character::StartLeftClick()
{
	holdingLeftClick = true;
}

void ARevisionGame4Character::StopLeftClick()
{
	holdingLeftClick = false;
}

void ARevisionGame4Character::StartRightClick()
{
	holdingRightClick = true;
}

void ARevisionGame4Character::StopRightClick()
{
	holdingRightClick = false;
	pulledActors.Empty();
	grappledActor = nullptr;
	isGrappling = false;
}

void ARevisionGame4Character::Scroll(float Value)
{
	scrollVal = Value;
}

void ARevisionGame4Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ARevisionGame4Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool ARevisionGame4Character::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ARevisionGame4Character::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ARevisionGame4Character::EndTouch);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////Player Functions

void ARevisionGame4Character::Hover(float DeltaTime)
{
	if (isHovering)
		GetCharacterMovement()->Velocity += FVector(0.0f, 0.0f, hoverForce) * DeltaTime;
}

void ARevisionGame4Character::Select(float DeltaTime)
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
	float Reach = 10000.f;
	FVector LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;

	// Set parameters to use line tracing
	FHitResult Hit;
	FCollisionQueryParams TraceParams(FName(TEXT("")), true, GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

	// Raycast out to this distance
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		PlayerViewPointLocation,
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);

	// See what if anything has been hit and return what
	AActor* ActorHit = Hit.GetActor();

	if (ActorHit && ActorHit->ActorHasTag("Grapple"))
	{
		interactable = true;
		if (holdingRightClick && !isGrappling)
		{
			grappledActor = ActorHit;
		}

		if (!ActorHit->ActorHasTag("MovableGrapple"))
			return;
	}

	if (ActorHit && ActorHit->IsRootComponentMovable())
	{
		interactable = true;

		if (holdingRightClick && !pulledActors.Contains(ActorHit))
			pulledActors.Add(ActorHit);
	}
	else
	{
		interactable = false;
	}

}

void ARevisionGame4Character::Pull(float DeltaTime, int i, FVector target)
{

	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(pulledActors[i]->GetRootComponent());
	FVector newVel = MeshRootComp->GetPhysicsLinearVelocity();

	//Desired Velocity
	FVector desiredVel = target - pulledActors[i]->GetActorLocation();
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

void ARevisionGame4Character::Catch(float DeltaTime, int i, FVector target)
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

void ARevisionGame4Character::Throw(float DeltaTime)
{
	if (holdingLeftClick && caughtActors.Num() != 0)
	{
		if (TKCharge < TKChargeMax)
		{
			UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[caughtActors.Num() - 1]->GetRootComponent());
			TKCharge += (TKChargeRate / (MeshRootComp->GetMass() * 0.25)) * DeltaTime;
		}

		throwCount += scrollVal;

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

			if (caughtActors[i]->ActorHasTag("Platform"))
			{
				desiredVel = GetActorLocation() - caughtActors[i]->GetActorLocation();
			}
			else
			{
				desiredVel = frontTarget->GetComponentLocation() - caughtActors[i]->GetActorLocation();
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

	if (!holdingLeftClick && throwCount > 0)
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
		FVector LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;

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
				dir = Hit.ImpactPoint - caughtActors[i]->GetActorLocation();
			else
				dir = LineTraceEnd - caughtActors[i]->GetActorLocation();

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

void ARevisionGame4Character::Grapple(float DeltaTime)
{
	if (grappledActor == nullptr)
		return;

	isGrappling = true;

	FVector newVel = GetCharacterMovement()->Velocity;

	//Desired Velocity
	FVector desiredVel = grappledActor->GetActorLocation() - GetActorLocation();
	desiredVel *= maxTKGrappleSpeed;

	//Steering Force
	FVector steering = desiredVel - GetCharacterMovement()->Velocity;
	steering.Normalize();
	steering /= GetCharacterMovement()->Mass;

	//Add to velocity
	newVel += steering * 45000.0f * DeltaTime * maxTKGrappleSpeed;

	//MeshRootComp->SetPhysicsLinearVelocity(newVel);
	GetCharacterMovement()->Velocity = newVel;
}

void ARevisionGame4Character::Follow(float DeltaTime, int i, FVector target)
{
	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[i]->GetRootComponent());
	FVector newVel = MeshRootComp->GetPhysicsLinearVelocity();

	//Desired Velocity
	FVector desiredVel = target - caughtActors[i]->GetActorLocation();
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
