// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "RevisionGame4Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UCLASS(config = Game)
class ARevisionGame4Character : public ACharacter
{
	GENERATED_BODY()

		/** Pawn mesh: 1st person view (arms; seen only by self) */
		UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FirstPersonCameraComponent;

public:
	ARevisionGame4Character();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnUseItem OnUseItem;



protected:
	/** Fires a projectile. */
	void OnPrimaryAction();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	bool isDashing = false;
	bool canDash = false;
	bool holdingLeftClick = false;
	bool holdingRightClick = false;
	void StartDash();
	void StopDash();
	void StartLeftClick();
	void StopLeftClick();
	void StartRightClick();
	void StopRightClick();
	void Scroll(float Value);

	void Dash(float DeltaTime);
	void Select(float DeltaTime);
	void Pull(float DeltaTime, int i, FVector target);
	void Follow(float DeltaTime, int i, FVector target);
	void Catch(float DeltaTime, int i, FVector target);
	void Throw(float DeltaTime);
	void Grapple(float DeltaTime);

	void GlowObject();
	void UnglowObject();

	UPrimitiveComponent* grappledActor;
	TArray<UPrimitiveComponent*> pulledActors;
	TArray<UPrimitiveComponent*> caughtActors;
	TArray<UPrimitiveComponent*> caughtActorsToRemove;
	int throwCount = 0;
	int scrollVal = 0;
	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

	//Player properties
	UPROPERTY(EditAnywhere)
		float dashForce;
	UPROPERTY(EditAnywhere)
		float hoverForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float massLimit;
	UPROPERTY(EditAnywhere)
		float pickupRange;
	UPROPERTY(EditAnywhere)
		float TKPushForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TKChargeMax;
	UPROPERTY(EditAnywhere)
		float TKChargeRate;
	UPROPERTY(EditAnywhere)
		float health;
	UPROPERTY(EditAnywhere)
		float maxTKPullSpeed;
	UPROPERTY(EditAnywhere)
		float maxTKGrappleSpeed;
	UPROPERTY(EditAnywhere)
		float catchRadius;
	UPROPERTY(EditAnywhere) 
		float dashTimerMax = 0.1f; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float dashCooldownMax = 1.0f;
	UPROPERTY(BlueprintReadOnly)
	float dashCooldown = dashCooldownMax;
	USceneComponent* backTarget;
	USceneComponent* frontTarget;

	



	//Movement Speed
	//Jump Speed



protected:
	float dashTimer = dashTimerMax;
	bool jumpStatePrevFrame = false;
	bool dashStopped = true;
	bool movingForward = true;
	FVector LineTraceEnd;
	FVector3d velBeforeDash;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TKCharge = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool interactable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isGrappling = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float massTotal;
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/*
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so
	 *
	 * @param	InputComponent	The input component pointer to bind controls to 
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

