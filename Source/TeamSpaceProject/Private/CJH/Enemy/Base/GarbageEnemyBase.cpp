// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/GarbageEnemyBase.h"

AGarbageEnemyBase::AGarbageEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGarbageEnemyBase::BeginPlay()
{
	Super::BeginPlay();

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

void AGarbageEnemyBase::SetInfo()
{
	Size = FMath::RandRange(_spawnedInfo.MinSize, _spawnedInfo.MaxSize);

	// 적 크기 구조체에 따라 크기 설정
	FVector NewScale = FVector(Size, Size, Size);

	SetActorScale3D(NewScale);

	// 적 크기에 비례하여 능력치 증감
	_spawnedInfo.Max_HP *= Size;
	_spawnedInfo.Attack_Damage *= Size;
	_spawnedInfo.Value *= Size;

	_spawnedInfo.Current_HP = _spawnedInfo.Max_HP;
}

void AGarbageEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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