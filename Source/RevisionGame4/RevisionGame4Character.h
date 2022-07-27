// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Creature.h"
#include "PacifyTriggerVolume.h"

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
class ARevisionGame4Character : public ACreature
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

	bool isDashing = false;
	bool canDash = false;
	bool holdingLeftClick = false;
	bool holdingRightClick = false;
	FVector3d dashVec = FVector3d(0.0f);


protected:
	/** Fires a projectile. */
	void OnPrimaryAction();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	void Select(float DeltaTime) override;
	void StartDash();
	void StopDash();
	void StartPacify();
	void StopPacify();
	void StartLeftClick();
	void StopLeftClick();
	void StartRightClick();
	void StopRightClick();
	void Scroll(float Value);
	int scrollVal = 0;
	bool isPacifying = false;
	bool isEPressed = false;

	UPROPERTY(EditAnywhere)
		float dashForce;
	UPROPERTY(EditAnywhere)
		float dashTimerMax = 0.1f;

	UPROPERTY(EditAnywhere)
		float aimTimeSpeed= 0.5f;
	UPROPERTY(EditAnywhere)
		float aimDrainRate = 1.0f;

	//Energy Meter Stuff
	UPROPERTY(EditAnywhere)
	float energyMeterUnitAmt = 5.0f;
	UPROPERTY(EditAnywhere)
	float energyMeterRechargeRate = 1.0f;
	UPROPERTY(BlueprintReadOnly)
	float energyMeterMax = energyMeterUnitAmt;
	UPROPERTY(BlueprintReadOnly)
	float energyMeter = energyMeterMax;

	UPROPERTY(EditAnywhere)
		float grappleDrainRate = 0.5f;

	TArray<AActor*> actorsToPacify;


	void EnergyMeter(float DeltaTime);
	void Dash(float DeltaTime);
	FVector3d velBeforeDash;
	bool dashStopped = true;
	float dashTimer = dashTimerMax;
	void Grapple(float DeltaTime);
	void Catch(float DeltaTime, int i, FVector target) override;
	void Throw(float DeltaTime) override;
	void Pacify(float DeltaTime);

	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	void GlowObject();
	void UnglowObject();

	
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

protected:

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

