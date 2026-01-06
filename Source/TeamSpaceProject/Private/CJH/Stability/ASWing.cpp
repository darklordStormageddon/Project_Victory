// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASWing.h"

#include "CJH/Stability/ASManager.h"

#include "Kismet/GameplayStatics.h"

// Called when the game starts or when spawned
void AASWing::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle UnusedHandle;

	// 약간의 딜레이를 주고 날개 스폰 함수 호출
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AASWing::Spawn_Wing, 0.1f, false);
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
	// 날개의 바운드 구하기
	GetActorBounds(true, WingOrigin, WingExtent);

	// 날개의 반지름 계산
	float WingRadius = WingExtent.X;

	// 날개 부착 거리 계산
	float AttachDist = WingRadius * 2;

	if (Direction)
	{
		// 우측 날개 스폰
		AASWing* WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			GetActorLocation(),
			GetActorRotation(),
			RestWing,
			Numbering,
			true
		);

		if (!WingActor)
			return;

		//x값만 AttachDist만큼 이동
		FVector LocalOffset(AttachDist, 0, 0);
		FVector WorldOffset = GetActorTransform().TransformVector(LocalOffset);
		WingActor->SetActorLocation(GetActorLocation() + WorldOffset);

		// 상위 날개에 부착
		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		// 좌측 날개 스폰
		AASWing* WingActor = Manager->Artifical_Satellite_Wing_Spawn(
			GetActorLocation(),
			GetActorRotation(),
			RestWing,
			Numbering,
			false
		);

		if (!WingActor)
			return;

		//x값만 AttachDist만큼 이동
		FVector LocalOffset(AttachDist, 0, 0);
		FVector WorldOffset = GetActorTransform().TransformVector(LocalOffset);
		WingActor->SetActorLocation(GetActorLocation() - WorldOffset);

		// 상위 날개에 부착
		WingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

}

