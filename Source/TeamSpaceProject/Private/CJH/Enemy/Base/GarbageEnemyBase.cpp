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
	if (!HasAuthority())
		return;

	_targetInfo.Size = FMath::RandRange(_spawnedInfo.MinSize, _spawnedInfo.MaxSize);

	// 적 크기 구조체에 따라 크기 설정
	NewScale = FVector(_targetInfo.Size, _targetInfo.Size, _targetInfo.Size);

	Super::SetInfo();
}

void AGarbageEnemyBase::FollowOrbitTarget(float DeltaTime)
{
    if (!HasAuthority() || !bHasOrbitTarget)
        return;

    const FVector Current = GetActorLocation();
    const FVector ToTarget = OrbitTarget - Current;

    const float DistSq = ToTarget.SizeSquared();

    const float StopThreshold = 10.0f;
    const float StopThresholdSq = StopThreshold * StopThreshold;

    if (DistSq <= StopThresholdSq)
    {
        SetActorLocation(OrbitTarget);
        return;
    }

    const float Speed = 800.f;

    // 여기서만 sqrt 1번
    const float Dist = FMath::Sqrt(DistSq);
    const FVector Dir = ToTarget / Dist;

    const FVector MoveDelta = Dir * Speed * DeltaTime;

    AddActorWorldOffset(MoveDelta, true);

    const FRotator DesiredRot = Dir.Rotation();
    const FRotator NewRot = FMath::RInterpTo(
        GetActorRotation(),
        DesiredRot,
        DeltaTime,
        5.0f
    );

    SetActorRotation(NewRot);
}