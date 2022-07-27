// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Creature.generated.h"

UCLASS()
class REVISIONGAME4_API ACreature : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACreature();
	virtual void Select(float DeltaTime);
	void Pull(float DeltaTime, int i, FVector target);
	void Follow(float DeltaTime, int i, FVector target);
	virtual void Catch(float DeltaTime, int i, FVector target);
	virtual void Throw(float DeltaTime);
	void CatchingPulling(float DeltaTime);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	AActor* grappledActor;
	TArray<AActor*> pulledActors;
	TArray<AActor*> caughtActors;
	TArray<AActor*> caughtActorsToRemove;
	USceneComponent* backTarget;
	USceneComponent* frontTarget;

	//Creature Properties
	UPROPERTY(EditAnywhere)
		float health;
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
		float maxTKPullSpeed;
	UPROPERTY(EditAnywhere)
		float maxTKGrappleSpeed;
	UPROPERTY(EditAnywhere)
		float catchRadius;

	bool jumpStatePrevFrame = false;
	bool movingForward = true;
	FVector LineTraceEnd;
	int throwCount = 0;

	bool throwCharging = false;

public:	
	//Stuff Needed For UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TKCharge = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool interactable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isGrappling = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float massTotal;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
