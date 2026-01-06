// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASBody.h"

#include "CJH/Stability/ASManager.h"

#include "Kismet/GameplayStatics.h"


// Called when the game starts or when spawned
void AASBody::BeginPlay()
{
	Super::BeginPlay();
	
	// 양쪽 첫번째 날개 스폰
	SpawnFirstWing();
}

void AASBody::SpawnFirstWing()
{
	AActor* SpawnManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!SpawnManager)
		return;

	Manager = Cast<AASManager>(SpawnManager);
	if (!Manager)
		return;

	// 날개 개수가 0개인 경우 리턴
	if (Manager->GetWingsNum() - 1 < 0)
		return;

	Call_WingSpawn();
}

void AASBody::Call_WingSpawn()
{
	// 랜덤으로 사용할 날개 선택
	WingsNum = FMath::RandRange(0, Manager->GetWingsNum() - 1);
	// 남은 날개 수 계산
	RestWing = Manager->CorrectWingNum();

	WingArrow(Direction::RightWing);// 우측 첫번째 날개 스폰
	WingArrow(Direction::LeftWing);// 좌측 첫번째 날개 스폰
}

void AASBody::WingArrow(Direction wArrow)
{
	AActor* SpawnManager = UGameplayStatics::GetActorOfClass(this, AASManager::StaticClass());
	if (!SpawnManager)
		return;

	Manager = Cast<AASManager>(SpawnManager);
	if (!Manager)
		return;

	FVector BodyOrigin, BodyExtent;

	// 본체의 바운드 구하기
	GetActorBounds(true, BodyOrigin, BodyExtent);

	// 본체 반지름 계산
	float BodyRadius = BodyExtent.X; 

	if(wArrow == Direction::RightWing)
	{
		// 우측 첫번째 날개 스폰
			WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			GetActorLocation(),
			GetActorRotation(),
			RestWing,
			WingsNum,
			true
		);

		if (!WingActor)
			return;

		//x값만 AttachDist만큼 이동
		FVector LocalOffset(AttachDist(BodyRadius), 0, 0);
		FVector WorldOffset = GetActorTransform().TransformVector(LocalOffset);
		WingActor->SetActorLocation(GetActorLocation() + WorldOffset);

		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		// 좌측 첫번째 날개 스폰
			WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			GetActorLocation(),
			GetActorRotation(),
			RestWing,
			WingsNum,
			false
		);
		
		if (!WingActor)
			return;

		//x값만 AttachDist만큼 이동
		FVector LocalOffset(AttachDist(BodyRadius), 0, 0);
		FVector WorldOffset = GetActorTransform().TransformVector(LocalOffset);
		WingActor->SetActorLocation(GetActorLocation() - WorldOffset);

		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

float AASBody::WingDist() 
{
	return FMath::RandRange(Manager->GetRandomFirstDist(true), Manager->GetRandomFirstDist(false)); // 날개와 본체 사이의 거리 랜덤 설정
}

float AASBody::AttachDist(float BodyRadius)
{
	FVector WingOrigin, WingExtent;

	if (!WingActor)
		return 0.f;

	// 날개의 바운드 구하기
	WingActor->GetActorBounds(true, WingOrigin, WingExtent);

	// 날개 반지름 계산
	float WingRadius = WingExtent.X;

	// 본체와 날개 사이의 거리 계산
	float AttachDist = BodyRadius + WingRadius + WingDist();

	// 계산된 거리 반환
	return AttachDist;
}