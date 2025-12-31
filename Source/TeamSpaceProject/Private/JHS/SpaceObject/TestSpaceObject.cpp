// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/TestSpaceObject.h"

ATestSpaceObject::ATestSpaceObject()
{
	PrimaryActorTick.bCanEverTick = true;
}

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

	if (!StartPoint || !EndPoint)
	{
		return;
	}

	_currentMovementTime += DeltaTime;

	float _alpha = FMath::Clamp(_currentMovementTime / MovementDuration, 0.0f, 1.0f);

	FVector _nextLocation = FMath::Lerp(_startMovementLocation, _endMovementLocation, _alpha);

	FRotator _nextRotator = FMath::Lerp(_startMovementRotator, _endMovementRotator, _alpha);

	if (YawRotationSpeed != 0.0f)
	{
		float _yawDelta = YawRotationSpeed * DeltaTime;
		_accumulatedYaw += _yawDelta;
		_nextRotator.Yaw += _accumulatedYaw;
	}

	SetActorLocation(_nextLocation);
	SetActorRotation(_nextRotator);

	FVector _targetLocation = _isMovingToEnd ? EndPoint->GetActorLocation() : StartPoint->GetActorLocation();
	float _distanceToTarget = FVector::Dist(_nextLocation, _targetLocation);

	if (_distanceToTarget <= ReachDistance || _currentMovementTime >= MovementDuration)
	{
		SetActorLocation(_targetLocation);
		FRotator _targetRotator = _isMovingToEnd ? EndPoint->GetActorRotation() : StartPoint->GetActorRotation();
		if (YawRotationSpeed != 0.0f)
		{
			_targetRotator.Yaw += _accumulatedYaw;
		}
		SetActorRotation(_targetRotator);

		_isMovingToEnd = !_isMovingToEnd;

		StartMovement();
	}
}

void ATestSpaceObject::StartMovement()
{
	_currentMovementTime = 0.0f;

	_startMovementLocation = GetActorLocation();
	_startMovementRotator = GetActorRotation();

	if (_isMovingToEnd)
	{
		_endMovementLocation = EndPoint->GetActorLocation();
		_endMovementRotator = EndPoint->GetActorRotation();
	}
	else
	{
		_endMovementLocation = StartPoint->GetActorLocation();
		_endMovementRotator = StartPoint->GetActorRotation();
	}
}