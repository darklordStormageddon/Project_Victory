// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASBody.h"

#include "CJH/Stability/ASManager.h"

#include "Kismet/GameplayStatics.h"


// Sets default values
AASBody::AASBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AASBody::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnFirstWing();
}

// Called every frame
void AASBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASBody::SpawnFirstWing()
{
	AActor* SpawnManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!SpawnManager)
		return;

	AASManager* Manager;

	Manager = Cast<AASManager>(SpawnManager);
	if (!Manager)
		return;

	if (Manager->GetWingsNum() - 1 < 0)
		return;

	WingsNum = FMath::RandRange(0, Manager->GetWingsNum() - 1);
	RestWing = Manager->CorrectWingNum();

	WingArrow(Direction::RightWing);
	WingArrow(Direction::LeftWing);
}

void AASBody::WingArrow(Direction wArrow)
{
	AActor* SpawnManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!SpawnManager)
		return;

	AASManager* Manager;

	Manager = Cast<AASManager>(SpawnManager);
	if (!Manager)
		return;

	FVector BodyOrigin, BodyExtent;
	GetActorBounds(true, BodyOrigin, BodyExtent);

	float BodyRadius = BodyExtent.X; 

	

	if(wArrow == Direction::RightWing)
	{
			WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			this->GetActorLocation() + AttachDist(BodyRadius),
			this->GetActorRotation(),
			RestWing,
			WingsNum,
			true
		);

		if (!WingActor)
			return;

		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
			WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			this->GetActorLocation() - AttachDist(BodyRadius),
			this->GetActorRotation(),
			RestWing,
			WingsNum,
			false
		);

		if (!WingActor)
			return;

		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}
float AASBody::WingDist() { return FMath::RandRange(MinRandomDist, MaxRandomDist); }

float AASBody::AttachDist(float BodyRadius)
{
	FVector WingOrigin, WingExtent;

	if (!WingActor)
		return 0.f;

	WingActor->GetActorBounds(true, WingOrigin, WingExtent);

	float WingRadius = WingExtent.X;

	float AttachDist = BodyRadius + WingRadius + WingDist();

	return AttachDist;
}