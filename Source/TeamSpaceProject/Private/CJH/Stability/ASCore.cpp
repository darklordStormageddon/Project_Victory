// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASCore.h"

#include "CJH/Stability/ASManager.h"
#include "CJH/Stability/ASBody.h"
#include "CJH/Stability/ASWing.h"

#include "Kismet/GameplayStatics.h"

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
	
	// 본체 스폰 함수 호출
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

	// 스폰할 수 있는 본체 종류 수 가져오기
	int BodiesNum = GetManager->GetBodiesNum();
	if (BodiesNum <= 0)
		return;

	// 본체 종류 중 랜덤으로 하나 선택
	int Index = FMath::RandRange(0, BodiesNum - 1);

	// 폐기물 매니저한테 본체 스폰 함수 호출 요청
	AASBody* BodyActor = GetManager->Artifical_Satellite_Body_Spawn(
		this->GetActorLocation(),
		this->GetActorRotation(),
		Index
	);

	if(!BodyActor)
		return;

	// 스폰된 본체를 코어에 부착
	BodyActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

}


