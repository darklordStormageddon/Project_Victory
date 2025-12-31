// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASBody.h"
#include "CJH/Stability/ASWing.h"
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
	AActor* ActorManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!ActorManager)
		return;

	AASManager* Manager = Cast<AASManager>(ActorManager);
	if (!Manager)
		return;

	if (Manager->GetWingsNum() - 1 < 0)
		return;

	int WingsNum = FMath::RandRange(0, Manager->GetWingsNum() - 1);
	int RestWing = Manager->CorrectWingNum();
	for (int i = 0; i <= 1; i++)
	{
		AASWing* WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			this->GetActorLocation(),
			this->GetActorRotation(),
			RestWing,
			WingsNum
		);

		if (!WingActor)
			return;

		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}


float AASBody::WingDist() { return FMath::RandRange(MinRandomDist, MaxRandomDist); }
