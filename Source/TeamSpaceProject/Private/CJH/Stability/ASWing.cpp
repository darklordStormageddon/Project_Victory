// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASWing.h"

#include "CJH/Stability/ASManager.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AASWing::AASWing()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AASWing::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle UnusedHandle;

	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AASWing::Spawn_Wing, 0.1f, false);
}

// Called every frame
void AASWing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASWing::Spawn_Wing()
{
	AActor* ActorManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!ActorManager)
		return;

	AASManager* Manager = Cast<AASManager>(ActorManager);
	if (!Manager)
		return;

	FVector WingOrigin, WingExtent;
	GetActorBounds(true, WingOrigin, WingExtent);

	float WingRadius = WingExtent.X;

	float AttachDist = WingRadius * 2;

	if (Direction)
	{
		AASWing* WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			this->GetActorLocation() + AttachDist,
			this->GetActorRotation(),
			RestWing,
			Numbering,
			true
		);

		if (WingActor)
			WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{

		AASWing* WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			this->GetActorLocation() - AttachDist,
			this->GetActorRotation(),
			RestWing,
			Numbering,
			false
		);

		if (WingActor)
			WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

}

