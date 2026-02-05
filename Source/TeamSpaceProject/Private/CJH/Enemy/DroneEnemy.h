#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/GarbageEnemyBase.h"

#include "Components/ArrowComponent.h"

#include "DroneEnemy.generated.h"

class ABullet;

// ===== LOD 레벨 정의 =====
UENUM(BlueprintType)
enum class ELODLevel : uint8
{
	Close = 0,     // 0~250m (25,000 UU): 최고 품질
	Far = 1,       // 250m~500m (25,000~50,000 UU): 중간 품질
	VeryFar = 2    // 500m 이상 (50,000 UU 이상): 최저 품질
};

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
	bool bIsChasing = false;

	// 스핀 속도 (도/초)
	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "0.0"))
	float SpinSpeed = 100.0f;

	// --- Chase & Orbit 관련 ---
	// 공전 곡선 기본 진폭(미터)
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveAmplitude = 300.0f;

	// 공전 곡선 진동빈도(회/초)
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "0.0"))
	float ChaseCurveFrequency = 0.5f;

	// 위상 업데이트: 각도(rad)
	float ChaseCurvePhase = 0.0f;

	// 위상 업데이트: 방향 부호(sign, ±1)
	float ChaseCurveSign = 1.0f;

	// 공전 추격 거리(Detail모드 한정)
	UPROPERTY(EditDefaultsOnly, Category = "Drone", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChaseDistance = 2000.0f;

	// 회전축 벡터
	FVector RotationAxis;

	bool bOrbiting = false;

	// 각속도 현재속도(라디안/초) 캐시/타겟, 천천한 보간으로 자연스러운 감소
	float AngularSpeedCurrent = 0.0f;
	float AngularSpeedTarget = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float AngularLerpSpeed = 1.0f; // 느린 가속 속도

	// 방향 변경 주기 & 확률
	float TimeSinceDirChange = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirChangeInterval = 4.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirReverseProbability = 0.25f;

	// 공전축이 아닌 수직 방향의 tilt 각도/타겟 값
	float TiltAngleCurrent = 0.0f;          // 현재 각도
	float TiltAngleTarget = 0.0f;           // 목표 각도
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float TiltAngleRange = 45.0f;           // 범위선 (도)

	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltLerpSpeed = 0.5f;             // 틸트 보간 속도(느림)

	float TiltAxisYaw = 0.0f;               // tilt axis를 정의하는 수평 Yaw (deg)
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltChangeInterval = 3.0f;        // tilt 타겟 변경 주기(sec)
	float TimeSinceTiltChange = 0.0f;

	// 진동하는 미세한 공전 (허브)
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscAmplitude = 5.0f;          // 진폭
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltOscFrequency = 0.2f;          // Hz

	float LoseTargetTime = 0.0f; 
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Target")
	float LoseTargetDelay = 1.2f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerTransform)
	FTransform ServerTransform;

	FTransform PrevTransform;
	float InterpAlpha;

	UFUNCTION()
	void OnRep_ServerTransform();

	// ===== LOD 관련 멤버 변수 =====
	UPROPERTY()
	ELODLevel CurrentLOD = ELODLevel::Close;

	FTimerHandle LODTimerHandle;

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

	// ===== LOD 함수 =====
	void UpdateLOD();
	void ApplyLODSettings();

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
