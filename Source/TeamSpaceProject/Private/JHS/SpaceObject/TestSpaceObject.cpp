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

	// ��ġ ����
	FVector _nextLocation = FMath::Lerp(_startMovementLocation, _endMovementLocation, _alpha);

	// ȸ�� ����
	FRotator _nextRotator = FMath::Lerp(_startMovementRotator, _endMovementRotator, _alpha);

	// y�� ȸ�� �߰�
	if (YawRotationSpeed != 0.0f)
	{
		float _yawDelta = YawRotationSpeed * DeltaTime;
		_accumulatedYaw += _yawDelta;
		_nextRotator.Yaw += _accumulatedYaw;
	}

	// ��ġ�� ȸ�� ����
	SetActorLocation(_nextLocation);
	SetActorRotation(_nextRotator);

	// ��ǥ ���������� �Ÿ� Ȯ��
	FVector _targetLocation = _isMovingToEnd ? EndPoint->GetActorLocation() : StartPoint->GetActorLocation();
	float _distanceToTarget = FVector::Dist(_nextLocation, _targetLocation);

	// ��ǥ ������ �����߰ų� �ð��� ��������
	if (_distanceToTarget <= ReachDistance || _currentMovementTime >= MovementDuration)
	{
		// ���� ��ġ�� ȸ�� ���� (yaw 회전은 유지)
		SetActorLocation(_targetLocation);
		FRotator _targetRotator = _isMovingToEnd ? EndPoint->GetActorRotation() : StartPoint->GetActorRotation();
		if (YawRotationSpeed != 0.0f)
		{
			_targetRotator.Yaw += _accumulatedYaw;
		}
		SetActorRotation(_targetRotator);

		// �̵� ���� ��ȯ
		_isMovingToEnd = !_isMovingToEnd;

		// �ݴ������� �̵� ����
		StartMovement();
	}
}

void ATestSpaceObject::StartMovement()
{
	_currentMovementTime = 0.0f;

	// ���� ��ġ�� ȸ���� ����
	_startMovementLocation = GetActorLocation();
	_startMovementRotator = GetActorRotation();

	// �̵� ���⿡ ���� ��ǥ ��ġ�� ȸ���� ����
	if (_isMovingToEnd)
	{
		// �������� �̵�
		_endMovementLocation = EndPoint->GetActorLocation();
		_endMovementRotator = EndPoint->GetActorRotation();
	}
	else
	{
		// ���������� �̵�
		_endMovementLocation = StartPoint->GetActorLocation();
		_endMovementRotator = StartPoint->GetActorRotation();
	}
}