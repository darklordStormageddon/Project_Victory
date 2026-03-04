#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "Components/ArrowComponent.h"
#include "DroneEnemy.generated.h"

class ABullet;

UCLASS()
class ADroneEnemy : public AGarbageEnemyBase
{
	GENERATED_BODY()

	// ── 컴포넌트 ──────────────────────────────────────────────
private:
	UPROPERTY(VisibleAnywhere, Category = "Base")
	UArrowComponent* MuzzleArrow;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UStaticMeshComponent* TurretMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<ABullet> Bullet;

	// ── 사격 ──────────────────────────────────────────────────
private:
	bool CanFire = true;
	FTimerHandle FireCooldownHandle;

	// ── 추격 상태 (서버 → 클라 복제) ─────────────────────────
private:
	UPROPERTY(Replicated)
	bool bIsChasing = false;

	// ── 추격 궤도 파라미터 ────────────────────────────────────
private:
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChaseDetectionMult = 1.0f;

	float ChaseCurvePhase = 0.0f;
	float OrbitAngle      = 0.0f; // 공전 각도 누적

	float AngularSpeedTarget = 0.0f;
	float AngularSpeedCurrent = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float AngularLerpSpeed = 2.0f;

	float TimeSinceDirChange = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirChangeInterval = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float DirReverseProbability = 0.25f;

	float TiltAngleCurrent = 0.0f;
	float TiltAngleTarget = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float TiltAngleRange = 30.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltLerpSpeed = 1.0f;

	float TiltAxisYaw = 0.0f;
	float TimeSinceTiltChange = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase|Tilt")
	float TiltChangeInterval = 3.0f;

	float LoseTargetTimer = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Chase")
	float LoseTargetDelay = 1.5f;

	// ── 네트워크 위치·회전 복제 ───────────────────────────────
	// RepLocation은 AGarbageEnemyBase에서 상속
protected:
	UPROPERTY(ReplicatedUsing = OnRep_DroneRot)
	FRotator RepRotation;

	UPROPERTY(ReplicatedUsing = OnRep_DroneState)
	FVector_NetQuantize RepDroneLoc;

	// 서버 공전 각도 복제 → 클라이언트 예측 동기화용
	UPROPERTY(Replicated)
	float RepOrbitAngle = 0.0f;

	// ── 서버 이동 타이머 ─────────────────────────────────────
	private:
		float ServerMoveDeltaTime = 0.05f; // 클라이언트 보간 기준 주기로만 사용

	// ── 클라이언트 보간 (VInterpTo 방식) ────────────────────────
private:
	FRotator ClientSmoothRot  = FRotator::ZeroRotator;
	FRotator ClientTargetRot  = FRotator::ZeroRotator;
	bool     bDroneClientInit = false;

	UPROPERTY(EditDefaultsOnly, Category = "Network", meta = (ClampMin = "200.0"))
	float SnapDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Network", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float ClientInterpSpeed = 8.0f;

	// ── 클라이언트 공전 예측 ──────────────────────────────────
private:
	float    ClientOrbitAngle    = 0.0f;  // 클라이언트 측 각도 누적
	bool     bClientOrbitSynced  = false; // 서버 각도 첫 동기화 완료 여부

	// ── 내부 함수 ─────────────────────────────────────────────
	private:
		void ServerMove(float DeltaTime);
		void ChaseMoveServer(float DeltaTime);
		void OrbitMoveServer(float DeltaTime);

		void LookAtTarget(float DeltaTime);
		void TryFire();
		void EnableFiring() { CanFire = true; }
		void CheckTarget();

		UFUNCTION()
		void OnRep_DroneState();

		UFUNCTION()
		void OnRep_DroneRot();

		UFUNCTION(NetMulticast, Unreliable)
		void Multicast_FireEffect();

	protected:
	ADroneEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void ClientTickInterp(float DeltaTime) override {}

	public:
	bool IsChasing() const { return bIsChasing; }
	void GetOwnerGarbage(AActor* InOwner) { _owner = InOwner; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};