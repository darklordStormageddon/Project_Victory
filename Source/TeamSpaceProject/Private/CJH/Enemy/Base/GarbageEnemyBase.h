// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/EnemyBase.h"
#include "GarbageEnemyBase.generated.h"

UCLASS()
class AGarbageEnemyBase : public AEnemyBase
{
	GENERATED_BODY()

private:
	FVector Center;
	float CenterAngle = 0.0f;

protected:
	AGarbageEnemyBase();

	// SpawnComponent가 계산한 궤도 위 위치를 서버에서 직접 설정
	// 클라이언트에는 RepLocation으로 복제
	UPROPERTY(ReplicatedUsing = OnRep_GarbageState)
	FVector_NetQuantize RepLocation;

	// 자전축·속도
	FVector RotationAxis;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation")
	float SpinSpeed = 50.0f;

	// 클라이언트 보간
	FVector ClientSmoothLoc   = FVector::ZeroVector;
	FVector ClientTargetLoc   = FVector::ZeroVector;
	FVector ClientPrevLoc     = FVector::ZeroVector;
	float   ClientInterpSpeed = 0.0f;  // 서버에서 받은 실제 궤도 접선 속도
	float   ClientInterpAlpha = 1.0f;
	bool    bClientLocInit    = false;

	UFUNCTION()
	void OnRep_GarbageState();

	virtual void BeginPlay() override;
	virtual void SetInfo() override;
	virtual void Tick(float DeltaTime) override;

public:
	// SpawnComponent가 0.05초마다 호출: 서버에서 액터를 궤도 위에 직접 배치
	// TangentSpeed: 실제 궤도 접선 속도 (클라이언트 보간 속도로 사용)
	void SetOrbitPosition(const FVector& InPos, float TangentSpeed = 0.0f);
	void SetSpinAxis(const FVector& NewAxis) { RotationAxis = NewAxis.GetSafeNormal(); }

	// 하위 호환
	void SetOrbitTarget(const FVector& InTarget) { SetOrbitPosition(InTarget, 0.0f); }
	void ClearOrbitTarget() {}
	void FollowOrbitTarget(float DeltaTime) {}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
