// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASCore.h"

#include "CJH/Stability/ASManager.h"
#include "CJH/Stability/ASBody.h"
#include "CJH/Stability/ASWing.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AASCore::AASCore()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AASCore::BeginPlay()
{
	Super::BeginPlay();

	AActor* ActorManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!ActorManager)
		return;

	Manager = Cast<AASManager>(ActorManager);
	if (!Manager)
		return;
	
	SpawnBody();
}

void AASCore::SpawnBody()
{
	AActor* ActorManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!ActorManager)
		return;

	AASManager* GetManager = Cast<AASManager>(ActorManager);
	if (!GetManager)
		return;

	int BodiesNum = GetManager->GetBodiesNum();
	if (BodiesNum <= 0)
		return;

	// 유효한 인덱스(0 .. BodiesNum-1)를 랜덤으로 선택
	int Index = FMath::RandRange(0, BodiesNum - 1);

	AASBody* BodyActor = GetManager->Artifical_Satellite_Body_Spawn(
		this->GetActorLocation(),
		this->GetActorRotation(),
		Index
	);

	if(!BodyActor)
		return;

	BodyActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

}


