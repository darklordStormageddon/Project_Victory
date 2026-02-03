// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/GarbageEnemyBase.h"

#include "KSM/HealthComponent.h"

AGarbageEnemyBase::AGarbageEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGarbageEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 초기 중심
		if (_owner)
			Center = _owner->GetActorLocation();
		else
			Center = GetActorLocation();

		CenterAngle = 0.0f;

		FTimerHandle DelayHandle;

		GetWorld()->GetTimerManager().SetTimer(
			DelayHandle,
			this,
			&AGarbageEnemyBase::SetInfo,
			0.01f,
			false
		);
	}
}

void AGarbageEnemyBase::SetInfo()
{
	_targetInfo.Size = FMath::RandRange(_spawnedInfo.MinSize, _spawnedInfo.MaxSize);

	// 적 크기 구조체에 따라 크기 설정
	NewScale = FVector(_targetInfo.Size, _targetInfo.Size, _targetInfo.Size);

	Super::SetInfo();
}

void AGarbageEnemyBase::FollowOrbitTarget(float DeltaTime)
{
	if (!bHasOrbitTarget) return;

	// 목표 위치까지의 벡터
	FVector Current = GetActorLocation();
	FVector ToTarget = OrbitTarget - Current;
	float Dist = ToTarget.Size();

	// 소형 보정: 너무 가까우면 정지
	const float StopThreshold = 10.0f;
	if (Dist <= StopThreshold)
	{
		// 정확히 고정
		SetActorLocation(OrbitTarget);
		return;
	}

	float Speed = 800.f;

	FVector MoveDelta = ToTarget.GetSafeNormal() * Speed * DeltaTime;

	// 충돌을 고려한 이동(충돌 허용)
	AddActorWorldOffset(MoveDelta, true);

	// 방향 회전(선택): 이동 방향을 향하도록 천천히 회전
	FRotator DesiredRot = ToTarget.Rotation();
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), DesiredRot, DeltaTime, 5.0f);
	SetActorRotation(NewRot);
}