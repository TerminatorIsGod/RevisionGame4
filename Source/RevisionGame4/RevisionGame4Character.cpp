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
	EnergyMeter(DeltaTime);
	Dash(DeltaTime);
	Select(DeltaTime);
	CatchingPulling(DeltaTime);
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

	PlayerInputComponent->BindAction("Hover", IE_Pressed, this, &ARevisionGame4Character::StartDash);
	PlayerInputComponent->BindAction("Hover", IE_Released, this, &ARevisionGame4Character::StopDash);

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

	dashVec += (LineTraceEnd - GetActorLocation()) * Value;

}

void ARevisionGame4Character::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}

	dashVec += GetActorRightVector() * Value;

}

void ARevisionGame4Character::StartDash()
{
	isDashing = true;
}

void ARevisionGame4Character::StopDash()
{
	isDashing = false;
	
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
	if (Value != 0) {
		UnglowObject();
		GlowObject();
	}
	
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

void ARevisionGame4Character::EnergyMeter(float DeltaTime)
{
	if (GetCharacterMovement()->IsMovingOnGround() && energyMeter < energyMeterMax)
	{
		energyMeter += DeltaTime * energyMeterRechargeRate;
	}

}

void ARevisionGame4Character::Dash(float DeltaTime)
{

	if (energyMeter >= energyMeterUnit && !isDashing && dashStopped)
		canDash = true;	
	
	//Start Dash
	if (isDashing && canDash)  
	{
		dashVec.Normalize();

		velBeforeDash = GetCharacterMovement()->Velocity;
		GetCharacterMovement()->Velocity += dashVec * dashForce * DeltaTime;

		energyMeter -= energyMeterUnit;
		canDash = false;

		dashTimer = dashTimerMax;
		dashStopped = false;
	}

	//Stop Dash
	if (dashTimer >= 0.0f)
		dashTimer -= DeltaTime;
	else if (!dashStopped)
	{
		GetCharacterMovement()->Velocity = (GetCharacterMovement()->Velocity *0.07f) + (velBeforeDash * 0.75f);
		dashStopped = true;
	}

	jumpStatePrevFrame = GetCharacterMovement()->IsMovingOnGround();
	dashVec = FVector3d(0.0f);
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
	LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;

	// Set parameters to use line tracing
	FHitResult Hit;
	FCollisionQueryParams TraceParams(FName(TEXT("")), false, GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

	// Raycast out to this distance
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		PlayerViewPointLocation,
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);

	// See what if anything has been hit and return what
	UPrimitiveComponent* ActorHit = Hit.GetComponent();



	if (ActorHit && Hit.GetActor()->ActorHasTag("Grapple"))
	{
		interactable = true;
		if (holdingRightClick && !isGrappling)
		{
			grappledActor = ActorHit;
		}

		if (!Hit.GetActor()->ActorHasTag("MovableGrapple"))
			return;
	}

	if (ActorHit && Hit.GetActor()->IsRootComponentMovable())
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

void ARevisionGame4Character::Grapple(float DeltaTime)
{
	if (grappledActor == nullptr)
		return;

	if (energyMeter <= 0.0f)
	{
		grappledActor = nullptr;
		isGrappling = false;
		return;
	}

	energyMeter -= DeltaTime * grappleDrainRate;

	isGrappling = true;
	UStaticMeshComponent* grappleMesh = Cast<UStaticMeshComponent>(grappledActor);
	FVector newVel = GetCharacterMovement()->Velocity;

	//Desired Velocity
	FVector desiredVel = grappleMesh->GetComponentLocation() - GetActorLocation();
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

void ARevisionGame4Character::Catch(float DeltaTime, int i, FVector target)
{
	UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(pulledActors[i]);
	FVector dir = target - MeshRootComp->GetComponentLocation();
	float dist = dir.Size();

	if (dist < catchRadius && MeshRootComp->GetMass() + massTotal <= massLimit)
	{
		if (!caughtActors.Contains(pulledActors[i]))
		{
			UnglowObject();
			caughtActors.Add(pulledActors[i]);
			GlowObject();
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
			UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[caughtActors.Num() - 1]);
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
			UStaticMeshComponent* MeshRootComp = Cast<UStaticMeshComponent>(caughtActors[i]);
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
		LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;

		// Set parameters to use line tracing
		FHitResult Hit;
		FCollisionQueryParams TraceParams(FName(TEXT("")), true, GetOwner());  // false to ignore complex collisions and GetOwner() to ignore self

		// Raycast out to this distance
		GetWorld()->LineTraceSingleByObjectType(
			OUT Hit,
			PlayerViewPointLocation,
			LineTraceEnd, 
			FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic), //Makes it so ray only intercects with static geometry. Might need to make this intercect with anything so it aims at enemies properly.
			TraceParams
		);

		for (int i = caughtActors.Num() - 1; i >= (caughtActors.Num() - throwCount); i--)
		{
			FVector dir;
			UStaticMeshComponent* actorToThrow = Cast<UStaticMeshComponent>(caughtActors[i]);

			if (Hit.ImpactPoint != FVector3d(0.0f))
				dir = Hit.ImpactPoint - actorToThrow->GetComponentLocation();
			else
				dir = LineTraceEnd - actorToThrow->GetComponentLocation();

			dir.Normalize();
			actorToThrow->SetPhysicsLinearVelocity(dir * TKCharge);
			massTotal -= actorToThrow->GetMass();
		}

		UnglowObject();

		for (int i = 0; i < throwCount; i++)
		{
			caughtActors.Pop();
		}

		throwCount = 0;
		TKCharge = 0;

		GlowObject();
	}
}

void ARevisionGame4Character::GlowObject() {
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("GlowObject called"));
	
	float posNum = caughtActors.Num();
	if (caughtActors.Num() >= (throwCount + 1)) {
		posNum = throwCount + 1;
	}

	if (caughtActors.Num() > 0) {
		for (int i = caughtActors.Num() - 1; i >= (caughtActors.Num() - posNum); i--) { //int i = caughtActors.Num() - 1; i >= (caughtActors.Num() - (throwCount + 1)); i--  //int i = 0; i < (throwCount + 1); i++
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Glowing Object +1"));
			UPrimitiveComponent* primComp = caughtActors[i];
			primComp->SetRenderCustomDepth(true);
		}
	}

}

void ARevisionGame4Character::UnglowObject() {
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("UnglowObject called"));
	if (caughtActors.Num() > 0) {
		for (int i = 0; i < caughtActors.Num(); i++) {
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Unglowing Object -1"));
			UPrimitiveComponent* primComp = caughtActors[i];
			primComp->SetRenderCustomDepth(false);
		}
	}

}
