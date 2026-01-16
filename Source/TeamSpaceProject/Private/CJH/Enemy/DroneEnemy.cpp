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

		if (DistanceCheck(_spawnedInfo.Attack_Range + 300))
			Fire();
	}
	else
		FollowOrbitTarget(DeltaTime);
}

void ADroneEnemy::ChaseMove(float DeltaTime)
{
	if (!Target)
		return;

	FVector TargetLoc = Target->GetActorLocation();
	FVector CurrentLoc = GetActorLocation();

	FVector DirFromTarget = (CurrentLoc - TargetLoc).GetSafeNormal();
	if (DirFromTarget.IsNearlyZero())
	{
		DirFromTarget = FVector::ForwardVector;
	}

	const FVector ApproachPoint =
		TargetLoc + DirFromTarget * (_spawnedInfo.Attack_Range - ApproachSpare);

	if (!bOrbiting)
	{
		GoToTarget(CurrentLoc, ApproachPoint, DeltaTime);
		return;
	}

	AngularSpeedCurrent = FMath::FInterpTo(
		AngularSpeedCurrent,
		AngularSpeedTarget,
		DeltaTime,
		AngularLerpSpeed
	);

	TimeSinceDirChange += DeltaTime;
	if (TimeSinceDirChange >= DirChangeInterval)
	{
		TimeSinceDirChange = 0.0f;

		float DegPerSec = FMath::FRandRange(6.0f, 28.0f);
		float Sign = (FMath::FRand() < DirReverseProbability) ? -1.0f : 1.0f;

		AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * Sign;
	}

	TimeSinceTiltChange += DeltaTime;
	if (TimeSinceTiltChange >= TiltChangeInterval)
	{
		TimeSinceTiltChange = 0.0f;
		TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
		TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	}

	float TiltOsc =
		TiltOscAmplitude *
		FMath::Sin(2.0f * PI * TiltOscFrequency * TimeSinceTiltChange);

	TiltAngleCurrent = FMath::FInterpTo(
		TiltAngleCurrent,
		TiltAngleTarget + TiltOsc,
		DeltaTime,
		TiltLerpSpeed
	);

	ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
	ChaseCurvePhase = FMath::Fmod(ChaseCurvePhase, 2.0f * PI);

	float RadiusOsc =
		ChaseCurveAmplitude * 0.5f *
		FMath::Sin(ChaseCurvePhase * 0.5f + 0.3f);

	float DesiredRadius = FMath::Max(50.0f, MaintainDistance + RadiusOsc);

	FVector Axis =	RotationAxis.IsNearlyZero() ? FVector::UpVector : RotationAxis.GetSafeNormal();

	FVector Temp =
		FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f
		? FVector::RightVector
		: FVector::UpVector;

	FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
	FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

	FVector Radial = (Right * FMath::Cos(ChaseCurvePhase) + Forward * FMath::Sin(ChaseCurvePhase)).GetSafeNormal();

	FVector TiltAxis =	FRotator(0.0f, TiltAxisYaw, 0.0f).RotateVector(Right).GetSafeNormal();

	FQuat TiltQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));

	FVector OrbitDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

	FVector DesiredPos = TargetLoc + OrbitDir * DesiredRadius;

	float CurrentDistToTarget = FVector::Dist(CurrentLoc, TargetLoc);

	if (CurrentDistToTarget < (MaintainDistance - MaintainTolerance))
	{
		FVector PushDir = (CurrentLoc - TargetLoc).GetSafeNormal();
		DesiredPos = TargetLoc + PushDir * (MaintainDistance + FMath::Abs(ChaseCurveAmplitude) * 0.3f);
	}

	float Speed = _spawnedInfo.Move_Speed;
	if (Speed <= 0.0f) Speed = 100.0f;

	FVector ToDesired = DesiredPos - CurrentLoc;
	float DistToDesired = ToDesired.Size();

	if (DistToDesired > KINDA_SMALL_NUMBER)
	{
		float MaxStep = Speed * DeltaTime;
		FVector MoveDelta = (DistToDesired > MaxStep) ? ToDesired.GetSafeNormal() * MaxStep : ToDesired;

		AddActorWorldOffset(MoveDelta, true);
	}
}

void ADroneEnemy::GoToTarget(FVector CurrentLoc, FVector TargetLoc, float DeltaTime)
{
	float Dist = FVector::Dist(CurrentLoc, TargetLoc);

	float MinDist = _spawnedInfo.Attack_Range - ApproachSpare;
	float MaxDist = _spawnedInfo.Attack_Range + ApproachSpare;

	float Speed = _spawnedInfo.Move_Speed;

	FVector ToTarget = TargetLoc - CurrentLoc;
	FVector ToTargetDir = ToTarget.GetSafeNormal();

	if (Dist > MaxDist)
	{
		bOrbiting = false;

		FVector MoveStep = ToTargetDir * Speed * DeltaTime;
		AddActorWorldOffset(MoveStep, true);
		return;
	}

	if (Dist < MinDist)
	{
		bOrbiting = false;

		FVector PushBackStep = -ToTargetDir * Speed * DeltaTime;
		AddActorWorldOffset(PushBackStep, true);
		return;
	}

	if (!bOrbiting)
	{
		EnterOrbit(TargetLoc);
	}

	OrbitAroundTarget(TargetLoc, DeltaTime);
}

void ADroneEnemy::EnterOrbit(const FVector& TargetLoc)
{
	bOrbiting = true;

	float DegPerSec = FMath::FRandRange(10.0f, 25.0f);
	float Sign = FMath::RandBool() ? 1.0f : -1.0f;

	AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * Sign;

	// 현재 속도가 0에 가까우면 바로 목표로 시작
	if (FMath::IsNearlyZero(AngularSpeedCurrent))
	{
		AngularSpeedCurrent = AngularSpeedTarget;
	}

	TimeSinceDirChange = 0.0f;

	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);

	TimeSinceTiltChange = 0.0f;
}

void ADroneEnemy::OrbitAroundTarget(const FVector& TargetLoc, float DeltaTime)
{
	AngularSpeedCurrent = FMath::FInterpTo(
		AngularSpeedCurrent,
		AngularSpeedTarget,
		DeltaTime,
		AngularLerpSpeed
	);

	TimeSinceDirChange += DeltaTime;
	if (TimeSinceDirChange >= DirChangeInterval)
	{
		TimeSinceDirChange = 0.0f;

		if (FMath::FRand() < DirReverseProbability)
		{
			AngularSpeedTarget *= -1.0f;
		}
	}

	TimeSinceTiltChange += DeltaTime;
	if (TimeSinceTiltChange >= TiltChangeInterval)
	{
		TimeSinceTiltChange = 0.0f;
		TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
		TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	}

	TiltAngleCurrent = FMath::FInterpTo(
		TiltAngleCurrent,
		TiltAngleTarget,
		DeltaTime,
		TiltLerpSpeed
	);

	float TiltOsc =
		TiltOscAmplitude *
		FMath::Sin(GetWorld()->TimeSeconds * TWO_PI * TiltOscFrequency);

	float FinalTiltAngle = TiltAngleCurrent + TiltOsc;

	FVector OrbitAxis =
		FRotator(
			FinalTiltAngle,
			TiltAxisYaw,
			0.0f
		).RotateVector(FVector::UpVector);

	FVector Offset = GetActorLocation() - TargetLoc;

	float AngleDeg =
		FMath::RadiansToDegrees(AngularSpeedCurrent * DeltaTime);

	FVector RotatedOffset =
		Offset.RotateAngleAxis(AngleDeg, OrbitAxis);

	SetActorLocation(TargetLoc + RotatedOffset, true);
}

void ADroneEnemy::LookTarget(float DeltaTime)
{
	if (!Target) return;

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (ToTarget.IsNearlyZero())
		return;

	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = ToTarget.Rotation();

	float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);
	float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentRot.Pitch, TargetRot.Pitch);

	float DeadAngle = 0.3f; // degrees (0.2 ~ 0.5 추천)

	if (FMath::Abs(DeltaYaw) < DeadAngle && FMath::Abs(DeltaPitch) < DeadAngle)
		return;

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

	if (FireParticle)
	{
		FireComponent = UGameplayStatics::SpawnEmitterAttached(
			FireParticle,
			MuzzleArrow,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTargetIncludingScale,
			true
		);
	}

	ABullet* SpawnedBullet = GetWorld()->SpawnActor<ABullet>(
		Bullet,
		MuzzleLocation,
		MuzzleRotation,
		SpawnParams
	);

	if (SpawnedBullet)
	{
		FVector LaunchDirection = Target->GetActorLocation() - this->GetActorLocation();
		SpawnedBullet->GetTarget(LaunchDirection.GetSafeNormal());
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