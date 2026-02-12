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

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<ABullet> Bullet;

	bool CanFire = true;

	// Turret 회전 속도(도/초)
	float RotateSpeed = 50.0f;

	// 타켓을 추격 중인지 여부
	UPROPERTY(Replicated)
	bool bIsChasing = false;

	// 스핀 속도 (도/초)
	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "0.0"))
	float SpinSpeed = 100.0f;

	// --- Chase & Orbit 관련 ---
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveAmplitude = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveFrequency = 0.5f;

	UPROPERTY(Replicated)
	float ChaseCurvePhase = 0.0f;

	float ChaseCurveSign = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChaseDistance = 2000.0f;

	FVector RotationAxis;

	UPROPERTY(Replicated)
	bool bOrbiting = false;

	UPROPERTY(Replicated)
	float AngularSpeedCurrent = 0.0f;

	float AngularSpeedTarget = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float AngularLerpSpeed = 1.0f;

	float TimeSinceDirChange = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirChangeInterval = 4.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirReverseProbability = 0.25f;

	UPROPERTY(Replicated)
	float TiltAngleCurrent = 0.0f;

	float TiltAngleTarget = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float TiltAngleRange = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltLerpSpeed = 0.5f;

	UPROPERTY(Replicated)
	float TiltAxisYaw = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltChangeInterval = 3.0f;
	float TimeSinceTiltChange = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscAmplitude = 5.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscFrequency = 0.2f;

	float LoseTargetTime = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Target")
	float LoseTargetDelay = 1.2f;

	// 클라: 속도 기반 예측 이동
	FVector ClientVelocity = FVector::ZeroVector;
	FVector ClientTargetLoc = FVector::ZeroVector;
	FRotator ClientTargetRot = FRotator::ZeroRotator;
	bool bClientInitialized = false;

	// 서버 위치 보정용
	UPROPERTY(Replicated)
	FVector_NetQuantize ServerLocation;

	UPROPERTY(Replicated)
	FRotator ServerRotation;

private:
	void Move(float DeltaTime);
	void ChaseMove(float DeltaTime);

	void LookTarget();
	void GoToTarget(FVector CurrentLoc, FVector TargetLoc, FVector ApproachPoint, float DeltaTime);
	void EnterOrbit();
	void OrbitAroundTarget(const FVector& ApproachPoint, float DeltaTime);

	void Fire();
	void CheckChaseDistance();
	void ApplySpin(float DeltaTime);

	void EnableFiring() { CanFire = true; }

	// 클라이언트 로컬 이동 (ChaseMove의 공전 부분)
	void ClientChaseMove(float DeltaTime);
	void ClientCorrectPosition(float DeltaTime);

protected:
	// 서버 → 클라: 위치 + 속도
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FVector_NetQuantize RepLocation;

	UPROPERTY(Replicated)
	FRotator RepRotation;

	UPROPERTY(Replicated)
	FVector_NetQuantize RepVelocity;

	UFUNCTION()
	void OnRep_ServerState();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireEffect();

protected:
	ADroneEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void GetOwnerGarbage(AActor* _droneowner) { _owner = _droneowner; }
	bool IsChasing() { return bIsChasing; }

	// 스핀축 설정
	void SetSpinAxis(const FVector& NewAxis) { RotationAxis = NewAxis.GetSafeNormal(); }

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
};