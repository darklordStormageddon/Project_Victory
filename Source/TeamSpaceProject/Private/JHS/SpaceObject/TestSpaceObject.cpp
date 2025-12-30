// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/TestSpaceObject.h"

void ATestSpaceObject::BeginPlay()
{
	Super::BeginPlay();

	if (StartPoint && EndPoint)
	{
		StartMovement();
	}
}

void ATestSpaceObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestSpaceObject::MovementTick(float DeltaTime)
{
	if (!StartPoint || !EndPoint)
	{
		return;
	}

	_currentMovementTime += DeltaTime;
	float _alpha = FMath::Clamp(_currentMovementTime / MovementDuration, 0.0f, 1.0f);

	// 위치 보간
	FVector _nextLocation = FMath::Lerp(_startMovementLocation, _endMovementLocation, _alpha);

	// 회전 보간
	FRotator _nextRotator = FMath::Lerp(_startMovementRotator, _endMovementRotator, _alpha);

	// y축 회전 추가
	if (YawRotationSpeed != 0.0f)
	{
		float _yawDelta = YawRotationSpeed * DeltaTime;
		_nextRotator.Yaw += _yawDelta;
	}

	// 위치와 회전 적용
	SetActorLocation(_nextLocation);
	SetActorRotation(_nextRotator);

	// 목표 지점까지의 거리 확인
	FVector _targetLocation = _isMovingToEnd ? EndPoint->GetActorLocation() : StartPoint->GetActorLocation();
	float _distanceToTarget = FVector::Dist(_nextLocation, _targetLocation);

	// 목표 지점에 도달했거나 시간이 지났으면
	if (_distanceToTarget <= ReachDistance || _currentMovementTime >= MovementDuration)
	{
		// 최종 위치와 회전 설정
		SetActorLocation(_targetLocation);
		SetActorRotation(_isMovingToEnd ? EndPoint->GetActorRotation() : StartPoint->GetActorRotation());

		// 이동 방향 전환
		_isMovingToEnd = !_isMovingToEnd;

		// 반대편으로 이동 시작
		StartMovement();
	}
}

void ATestSpaceObject::StartMovement()
{
	_currentMovementTime = 0.0f;

	// 시작 위치와 회전값 설정
	_startMovementLocation = GetActorLocation();
	_startMovementRotator = GetActorRotation();

	// 이동 방향에 따라 목표 위치와 회전값 설정
	if (_isMovingToEnd)
	{
		// 끝점으로 이동
		_endMovementLocation = EndPoint->GetActorLocation();
		_endMovementRotator = EndPoint->GetActorRotation();
	}
	else
	{
		// 시작점으로 이동
		_endMovementLocation = StartPoint->GetActorLocation();
		_endMovementRotator = StartPoint->GetActorRotation();
	}
}