// Fill out your copyright notice in the Description page of Project Settings.


#include "PacifyTriggerVolume.h"
#include "DrawDebugHelpers.h"
#include "Engine/BrushShape.h"



APacifyTriggerVolume::APacifyTriggerVolume()
{
	OnActorBeginOverlap.AddDynamic(this, &APacifyTriggerVolume::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &APacifyTriggerVolume::OnOverlapEnd);

}

void APacifyTriggerVolume::activateTrigger(bool isActive)
{
	activated = isActive;
}

void APacifyTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

	//DrawDebugBox(GetWorld(), GetActorLocation(), Brush->Bounds.BoxExtent, FColor::Orange, true, -1, 0, 5);
}

void APacifyTriggerVolume::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (activated && OtherActor && (OtherActor != this))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Overlap Begin"));

		if (OtherActor->IsRootComponentMovable() && OtherActor->GetRootComponent()->IsSimulatingPhysics())
		{
			UStaticMeshComponent* actorToStop = Cast<UStaticMeshComponent>(OtherActor->GetRootComponent());
			actorToStop->SetPhysicsLinearVelocity(GetActorForwardVector() * 6000.0f);
		}
	}
}

void APacifyTriggerVolume::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Overlap Ended"));
	}	
}
