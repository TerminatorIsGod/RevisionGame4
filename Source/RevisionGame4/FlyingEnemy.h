// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Creature.h"
#include "Perception/AIPerceptionComponent.h"
#include "FlyingEnemy.generated.h"

UCLASS()
class REVISIONGAME4_API AFlyingEnemy : public ACreature
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFlyingEnemy();


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = AI)
		class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, Category = AI)
		class UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, Category = AI)
		class UAIPerceptionComponent* AIPerceptionComp;

private:

	UFUNCTION()
		void OnPlayerCaught(APawn* Pawn);
};
