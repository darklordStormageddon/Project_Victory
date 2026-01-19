#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/GarbageEnemyBase.h"

#include "Components/ArrowComponent.h"

#include "DroneEnemy.generated.h"

class ABullet;

UCLASS()
class ADroneEnemy : public AGarbageEnemyBase
{
private:
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UArrowComponent* MuzzleArrow;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UStaticMeshComponent* TurretMesh;

	FVector MuzzleLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<ABullet> Bullet;

	bool CanFire = true;

	// Turret 회전 속도(인터폴 용)
	float RotateSpeed = 50.0f;

	// 드론이 현재 추격 중인지 여부
	bool bIsChasing = false;

	// 자전 속도 (도/초)
	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "0.0"))
	float SpinSpeed = 100.0f;

	// --- Chase 곡선 관련 설정 ---
	// 곡선 추격 시 오프셋 진폭(월드 단위)
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveAmplitude = 300.0f;

	// 곡선 추격 주파수(회/초)
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveFrequency = 0.5f;

	// 내부 상태: 위상(rad)
	float ChaseCurvePhase = 0.0f;

	// 내부 상태: 좌우 방향(sign, ±1)
	float ChaseCurveSign = 1.0f;

	// 추격 시작 거리(Detail에서 조정 가능)
	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChaseDistance = 2000.0f;

	// 자전축 벡터
	FVector RotationAxis;

	bool bOrbiting = false;

	// 공전 각속도(라디안/초) 현재/목표, 천천히 보간하여 다양성 제공
	float AngularSpeedCurrent = 0.0f;
	float AngularSpeedTarget = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float AngularLerpSpeed = 1.0f; // 초당 전환 속도

	// 방향 전환 주기 및 확률
	float TimeSinceDirChange = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirChangeInterval = 4.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirReverseProbability = 0.25f;

	// 컴포넌트가 아닌 개별 드론이 tilt 각도/축을 가짐
	float TiltAngleCurrent = 0.0f;          // 도 단위
	float TiltAngleTarget = 0.0f;           // 도 단위
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float TiltAngleRange = 45.0f;           // ±범위 (도)

	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltLerpSpeed = 0.5f;             // 도 단위 보간 속도(초당)

	float TiltAxisYaw = 0.0f;               // tilt axis를 결정하는 월드 Yaw (deg)
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltChangeInterval = 3.0f;        // tilt 목표 변경 주기(sec)
	float TimeSinceTiltChange = 0.0f;

	// 약간의 진동 추가 (소량)
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscAmplitude = 5.0f;          // 도
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscFrequency = 0.2f;          // Hz

	float LoseTargetTime = 0.0f; 
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Target")
	float LoseTargetDelay = 1.2f;

private:
	void Move(float DeltaTime);
	void ChaseMove(float DeltaTime);

	void LookTarget(float DeltaTime);
	void GoToTarget(FVector CurrentLoc, FVector TargetLoc, FVector ApproachPoint, float DeltaTime);
	void EnterOrbit();
	void OrbitAroundTarget(const FVector& ApproachPoint, float DeltaTime);

	void Fire();
	void CheckChaseDistance();
	void ApplySpin(float DeltaTime);

	void EnableFiring() { CanFire = true; }

protected:
	ADroneEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void GetOwnerGarbage(AActor* _droneowner) { _owner = _droneowner; }
	bool IsChasing() { return bIsChasing; }

	// 자전축 세터
	void SetSpinAxis(const FVector& NewAxis) { RotationAxis = NewAxis.GetSafeNormal(); }
};
