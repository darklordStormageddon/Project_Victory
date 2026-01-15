#include "CJH/Enemy/DroneEnemy.h"

#include "CJH/Enemy/Weapon/Bullet.h"

#include "JHS/GameControl/JHSGameMode.h"

ADroneEnemy::ADroneEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	RootComponent = TurretMesh;

	MuzzleArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	MuzzleArrow->SetupAttachment(RootComponent);

	RotationAxis = FVector(0.0f, 0.0f, 1.0f);

	ChaseCurveAmplitude = 300.0f;
	ChaseCurveFrequency = 0.5f;
	ChaseCurvePhase = 0.0f;
	ChaseCurveSign = 1.0f;

	// 초기 angular speed (rad/s) 설정
	AngularSpeedCurrent = FMath::DegreesToRadians(15.0f);
	AngularSpeedTarget = AngularSpeedCurrent;

	// tilt 기본값
	TiltAngleCurrent = 0.0f;
	TiltAngleTarget = 0.0f;
	TiltLerpSpeed = 0.5f;
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	TimeSinceTiltChange = 0.0f;
	TiltOscAmplitude = 5.0f;
	TiltOscFrequency = 0.2f;
}

void ADroneEnemy::BeginPlay()
{
	Super::BeginPlay();

	ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);
	ChaseCurveSign = FMath::FRand() > 0.5f ? 1.0f : -1.0f;

	TimeSinceDirChange = 0.0f;

	// 초기 tilt 목표
	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	TimeSinceTiltChange = 0.0f;
}

void ADroneEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 추격 거리 확인
	CheckChaseDistance();

	// 체이싱 상태가 꺼지면 orbiting 상태 초기화
	if (!bIsChasing)
		bOrbiting = false;

	if (bIsChasing && Target)
	{
		ChaseMove(DeltaTime);
		LookTarget(DeltaTime);

		if (DistanceCheck(_spawnedInfo.Attack_Range))
			Fire();
	}
	else
	{
		FollowOrbitTarget(DeltaTime);
	}
}

void ADroneEnemy::ChaseMove(float DeltaTime)
{
	if (!Target) return;

	const FVector TargetLoc = Target->GetActorLocation();
	const FVector CurrentLoc = GetActorLocation();

	// Approach point (target으로부터 MaintainDistance + ApproachBuffer 떨어진 지점)
	FVector DirFromTarget = (CurrentLoc - TargetLoc).GetSafeNormal();
	if (DirFromTarget.IsNearlyZero())
		DirFromTarget = FVector::ForwardVector;

	FVector ApproachPoint = TargetLoc + DirFromTarget * (MaintainDistance + ApproachBuffer);

	float DistToApproach = FVector::Dist(CurrentLoc, ApproachPoint);

	// 아직 공전 상태가 아니라면 먼저 접근
	if (!bOrbiting)
	{
		float Speed = _spawnedInfo.Move_Speed;
		if (Speed <= 0.0f) Speed = 100.0f;

		FVector ToGoal = ApproachPoint - CurrentLoc;
		float Dist = ToGoal.Size();
		if (Dist > KINDA_SMALL_NUMBER)
		{
			FVector MoveDir = ToGoal / Dist;
			float MaxStep = Speed * DeltaTime;
			FVector MoveDelta = (Dist > MaxStep) ? MoveDir * MaxStep : ToGoal;
			AddActorWorldOffset(MoveDelta, true);
		}

		// 접근 완료 시 공전 시작
		const float EnterThreshold = 20.0f;
		if (DistToApproach <= EnterThreshold)
		{
			bOrbiting = true;

			float DegPerSec = FMath::FRandRange(8.0f, 25.0f);
			AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * (FMath::RandBool() ? 1.0f : -1.0f);
			TimeSinceDirChange = 0.0f;

			// 새 tilt 목표도 설정
			TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
			TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
			TimeSinceTiltChange = 0.0f;
		}
		return;
	}

	// --- Orbiting: 다양하게 회전하도록 처리 ---
	// Angular speed 보간
	AngularSpeedCurrent = FMath::FInterpTo(AngularSpeedCurrent, AngularSpeedTarget, DeltaTime, AngularLerpSpeed);

	// 주기적으로 방향/속도 변경 확률
	TimeSinceDirChange += DeltaTime;
	if (TimeSinceDirChange >= DirChangeInterval)
	{
		TimeSinceDirChange = 0.0f;
		if (FMath::FRand() < DirReverseProbability)
		{
			AngularSpeedTarget *= -1.0f;
		}
		float DegPerSec = FMath::FRandRange(6.0f, 28.0f);
		AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * (FMath::FRand() > 0.5f ? 1.0f : -1.0f);
	}

	// tilt 목표 주기적 변경
	TimeSinceTiltChange += DeltaTime;
	if (TimeSinceTiltChange >= TiltChangeInterval)
	{
		TimeSinceTiltChange = 0.0f;
		TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
		TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	}

	// tilt 위상 진동(oscillation)
	float TiltOsc = TiltOscAmplitude * FMath::Sin(2.0f * PI * TiltOscFrequency * TimeSinceTiltChange);

	// Tilt 보간 (도 단위)
	TiltAngleCurrent = FMath::FInterpTo(TiltAngleCurrent, TiltAngleTarget + TiltOsc, DeltaTime, TiltLerpSpeed);

	// phase 증가 (angular speed already in rad/s)
	ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
	if (ChaseCurvePhase > 2.0f * PI) ChaseCurvePhase -= 2.0f * PI;
	if (ChaseCurvePhase < -2.0f * PI) ChaseCurvePhase += 2.0f * PI;

	// radius with gentle oscillation for variety
	float RadiusOsc = ChaseCurveAmplitude * 0.5f * FMath::Sin(ChaseCurvePhase * 0.5f + 0.3f);
	float DesiredRadius = MaintainDistance + RadiusOsc;
	DesiredRadius = FMath::Max(50.0f, DesiredRadius);

	// compute planar basis
	const FVector Axis = RotationAxis.IsNearlyZero() ? FVector::UpVector : RotationAxis.GetSafeNormal();
	const FVector Temp = FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f ? FVector::RightVector : FVector::UpVector;
	FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
	FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

	// 기본 원형 방향
	float Angle = ChaseCurvePhase;
	FVector Radial = (Right * FMath::Cos(Angle) + Forward * FMath::Sin(Angle)).GetSafeNormal();

	// Tilt 축: 월드 Z 회전(TiltAxisYaw)을 적용한 Right 방향을 tilt 축으로 사용
	FVector TiltAxis = FRotator(0.0f, TiltAxisYaw, 0.0f).RotateVector(Right).GetSafeNormal();
	// TiltQuat: TiltAxis를 축으로 TiltAngleCurrent(도)를 회전
	FQuat TiltQuat = FQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));

	// Tilt 적용된 방향
	FVector OrbitDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

	FVector DesiredPos = TargetLoc + OrbitDir * DesiredRadius;

	// MaintainDistance 보정 (너무 가까워지면 밀어냄)
	float CurrentDistToTarget = FVector::Dist(CurrentLoc, TargetLoc);
	if (CurrentDistToTarget < (MaintainDistance - MaintainTolerance))
	{
		FVector PushDir = (CurrentLoc - TargetLoc).GetSafeNormal();
		DesiredPos = TargetLoc + PushDir * (MaintainDistance + FMath::Abs(ChaseCurveAmplitude) * 0.3f);
	}

	// 이동: 부드럽게 접근, AddActorWorldOffset으로 프레임 단위로 이동하여 순간이동 방지
	float Speed = _spawnedInfo.Move_Speed;
	if (Speed <= 0.0f) Speed = 100.0f;

	FVector ToDesired = DesiredPos - CurrentLoc;
	float DistToDesired = ToDesired.Size();
	if (DistToDesired > KINDA_SMALL_NUMBER)
	{
		FVector MoveDir = ToDesired / DistToDesired;
		float MaxStep = Speed * DeltaTime;
		FVector MoveDelta = (DistToDesired > MaxStep) ? MoveDir * MaxStep : ToDesired;
		AddActorWorldOffset(MoveDelta, true);
	}

	
}
void ADroneEnemy::LookTarget(float DeltaTime)
{
	if (!Target) return;

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (ToTarget.IsNearlyZero())
		return;

	const FRotator CurrentRot = GetActorRotation();
	const FRotator TargetRot = ToTarget.Rotation();

	// Yaw / Pitch 차이 계산 (최단 경로)
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);
	const float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentRot.Pitch, TargetRot.Pitch);

	// 🎯 Dead Zone (각도 차이가 이보다 작으면 회전 안 함)
	const float DeadAngle = 0.3f; // degrees (0.2 ~ 0.5 추천)

	if (FMath::Abs(DeltaYaw) < DeadAngle && FMath::Abs(DeltaPitch) < DeadAngle)
	{
		// 충분히 바라보고 있음 → 회전 중단
		return;
	}

	FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		TargetRot,
		DeltaTime,
		2.5f
	);

	SetActorRotation(NewRot);
}
void ADroneEnemy::Fire()
{
	if (!CanFire)
		return;
	if (!Bullet)
		return;

	MuzzleLocation = MuzzleArrow->GetComponentLocation();

	FRotator MuzzleRotation = MuzzleArrow->GetComponentRotation();
	FActorSpawnParameters SpawnParams;

	SpawnParams.Owner = this;

	SpawnParams.Instigator = GetInstigator();

	ABullet* SpawnedBullet = GetWorld()->SpawnActor<ABullet>(
		Bullet,
		MuzzleLocation,
		MuzzleRotation,
		SpawnParams
	);

	if (SpawnedBullet)
	{
		FVector LaunchDirection = MuzzleArrow->GetForwardVector();
		SpawnedBullet->GetTarget(MuzzleLocation + LaunchDirection * 1000.0f);
	}

	CanFire = false;

	FTimerHandle FireRateHandle;

	GetWorld()->GetTimerManager().SetTimer(
		FireRateHandle,
		this,
		&ADroneEnemy::EnableFiring,
		_spawnedInfo.Attack_Speed,
		false
	);
}

void ADroneEnemy::GetOwnerGarbage(AActor* _droneowner)
{
	_owner = _droneowner;
}

void ADroneEnemy::CheckChaseDistance()
{
	// 일정 거리 이내면 추격 시작 (기존 DistanceCheck 사용)
	if (DistanceCheck(_spawnedInfo.Detection_Range))
	{
		if (!bIsChasing)
		{
			ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);
			ChaseCurveSign = FMath::FRand() > 0.5f ? 1.0f : -1.0f;
		}
		bIsChasing = true;
		Target = _spaceShip;
	}
	else
	{
		bIsChasing = false;
		Target = nullptr;
		bOrbiting = false;
	}
}

void ADroneEnemy::ApplySpin(float DeltaTime)
{
	if (RotationAxis.IsNearlyZero())
		return;

	FRotator CurrentRotation = this->GetActorRotation();
	FVector CurrentAxis = CurrentRotation.RotateVector(RotationAxis);

	FQuat SpinQuat = FQuat(CurrentAxis, FMath::DegreesToRadians(SpinSpeed * DeltaTime));
	FQuat NewQuat = SpinQuat * this->GetActorQuat();

	this->SetActorRotation(NewQuat);
    
}