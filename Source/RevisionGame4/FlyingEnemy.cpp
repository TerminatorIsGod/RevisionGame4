// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyingEnemy.h"
#include "FlyingEnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/PawnSensingComponent.h"

// Sets default values
AFlyingEnemy::AFlyingEnemy()
{
	//Initialize Senses
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SetPeripheralVisionAngle(90.0f);
}

// Called when the game starts or when spawned
void AFlyingEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AFlyingEnemy::OnPlayerCaught);
	}

}

// Called to bind functionality to input
void AFlyingEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AFlyingEnemy::OnPlayerCaught(APawn* Pawn)
{ 
	//Get ref to player controller
	AFlyingEnemyController* AIController = Cast< AFlyingEnemyController>(GetController());

	if (AIController)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("PLAYER CAUGHT!"));
		AIController->SetPlayerCaught(Pawn);
	}
}

