// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/EnemyBase.h"
#include "GarbageEnemyBase.generated.h"

/**
 *
 */
UCLASS()
class AGarbageEnemyBase : public AEnemyBase
{
	GENERATED_BODY()
private:
	// 기존 필드
	FVector Center;
	float CenterAngle = 0.0f;


protected:
	AGarbageEnemyBase();

	// Orbit 관련(이 클래스는 단순히 목표를 따라가는 역할만 하도록 최소한의 API 제공)
	// 외부(컴포넌트)가 world 기준 목표를 계산해서 전달한다.
	FVector OrbitTarget;
	bool bHasOrbitTarget = false;

	// 자전축 벡터
	FVector RotationAxis;
	// 속도 등은 _spawnedInfo.Move_Speed 사용

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetInfo();

public:
	// 외부에서 호출: 컴포넌트가 계산한 월드 좌표 목표를 설정
	void SetOrbitTarget(const FVector& _target) { OrbitTarget = _target; bHasOrbitTarget = true; }

	// 목표 따라가기 (Tick에서 호출)
	void FollowOrbitTarget(float DeltaTime);

	// 목표 해제
	void ClearOrbitTarget() { bHasOrbitTarget = false; }
	
	// 외부에서 자전축 설정 가능하게 하는 세터
	void SetSpinAxis(const FVector& NewAxis) { RotationAxis = NewAxis.GetSafeNormal(); }
};
