#include "CJH/Enemy/DroneEnemy.h"

#include "CJH/Enemy/Weapon/Bullet.h"
#include "JHS/GameControl/JHSGameMode.h"

ADroneEnemy::ADroneEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 드론 본체 메쉬 (루트)
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	RootComponent = TurretMesh;

	// 총구 위치 표시용 Arrow
	MuzzleArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	MuzzleArrow->SetupAttachment(RootComponent);

	// 기본 회전 축 (Z축 기준 공전)
	RotationAxis = FVector(0.0f, 0.0f, 1.0f);

	// 공전 반경 진동 관련 파라미터
	ChaseCurveAmplitude = 300.0f;   // 반경 흔들림 최대치
	ChaseCurveFrequency = 0.5f;     // 진동 빈도
	ChaseCurvePhase = 0.0f;         // 위상
	ChaseCurveSign = 1.0f;          // 방향 (+ / -)

	// 공전 각속도 (rad/s)
	AngularSpeedCurrent = FMath::DegreesToRadians(15.0f); // 현재 각속도
	AngularSpeedTarget = AngularSpeedCurrent;             // 목표 각속도

	// 기울기(tilt) 관련 초기값
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

	// 공전 위상 랜덤 시작 (개체 간 동기화 방지)
	ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);
	ChaseCurveSign = FMath::FRand() > 0.5f ? 1.0f : -1.0f;

	TimeSinceDirChange = 0.0f;

	// 초기 기울기 랜덤
	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	TimeSinceTiltChange = 0.0f;
}

void ADroneEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 타겟 감지 / 추격 여부 판단
	CheckChaseDistance();

	// 추격이 꺼지면 공전 상태도 해제
	if (!bIsChasing)
		bOrbiting = false;

	if (bIsChasing && Target)
	{
		// 이동 로직
		ChaseMove(DeltaTime);

		// 타겟 바라보기
		LookTarget(DeltaTime);

		// 공격 사거리 내면 바로 발사
		if (DistanceCheck(_spawnedInfo.Attack_Range))
			Fire();
	}
	else
	{
		// 추격 대상 없으면 기본 오비트 행동
		FollowOrbitTarget(DeltaTime);
	}
}

void ADroneEnemy::ChaseMove(float DeltaTime)
{
	if (!Target)
		return;

	const FVector TargetLoc = Target->GetActorLocation();
	FVector CurrentLoc = GetActorLocation();

	// 거리 기준
	const float AttackR = _spawnedInfo.Attack_Range; // 주의: 실제 멤버 이름 프로젝트에 맞게 _spawnedInfo로 수정하세요
	const float OrbitMin = AttackR * 0.7f;
	const float OrbitMax = AttackR; // orbit 허용 구간 [OrbitMin, OrbitMax]

	float CurrentDistToTarget = FVector::Dist(CurrentLoc, TargetLoc);

	// 1) 너무 가까이 붙으면 부드럽게 백오프(OrbitMin까지)
	if (CurrentDistToTarget < OrbitMin)
	{
		// Back off 목표 : 타겟에서 OrbitMin 거리
		FVector BackDir = (CurrentLoc - TargetLoc).GetSafeNormal();
		if (BackDir.IsNearlyZero()) BackDir = FVector::ForwardVector;
		FVector BackTarget = TargetLoc + BackDir * OrbitMin;

		// 부드럽게 이동 (프레임 최대 이동량 제한)
		float Speed = FMath::Max(1.0f, _spawnedInfo.Move_Speed); // 프로젝트 멤버명 확인
		FVector ToBack = BackTarget - CurrentLoc;
		float Dist = ToBack.Size();
		if (Dist > KINDA_SMALL_NUMBER)
		{
			FVector Dir = ToBack / Dist;
			float MaxStep = Speed * DeltaTime;
			FVector Move = (Dist > MaxStep) ? Dir * MaxStep : ToBack;
			AddActorWorldOffset(Move, true);
		}
		// 계속 백오프하면 공전 상태 해제(혹은 유지하지 않음)
		bOrbiting = false;
		return;
	}

	// 2) 공격 사거리 밖이면 직접 접근(직진)
	if (CurrentDistToTarget > AttackR)
	{
		// 접근: target 쪽으로 직진
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _spawnedInfo.Move_Speed);
		float MaxStep = Speed * DeltaTime;
		AddActorWorldOffset(Dir * MaxStep, true);

		// 아직 공전 시작 조건 못 채우면 bOrbiting = false
		bOrbiting = false;
		return;
	}

	// 3) 현재 거리가 orbit 허용 구간(OrbitMin..OrbitMax) 안에 있으면 공전 유지 / 시작
	if (CurrentDistToTarget >= OrbitMin && CurrentDistToTarget <= OrbitMax)
	{
		// 보장: 공전 상태로 설정
		if (!bOrbiting)
		{
			EnterOrbit();
			// do not return: proceed to orbiting logic this frame
		}
		// 공전 이동 수행
		// compute desired orbit position then move towards it (to avoid teleport)
		// 공전 위상/tilt 계산 (재사용 기존 로직)
		AngularSpeedCurrent = FMath::FInterpTo(AngularSpeedCurrent, AngularSpeedTarget, DeltaTime, AngularLerpSpeed);

		// tilt 변경 주기 등 기존 로직 유지...
		TimeSinceDirChange += DeltaTime;
		if (TimeSinceDirChange >= DirChangeInterval)
		{
			TimeSinceDirChange = 0.0f;
			if (FMath::FRand() < DirReverseProbability)
				AngularSpeedTarget *= -1.0f;
			float DegPerSec = FMath::FRandRange(6.0f, 28.0f);
			float Sign = (FMath::FRand() > 0.5f) ? 1.0f : -1.0f;
			AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * Sign;
		}

		TimeSinceTiltChange += DeltaTime;
		if (TimeSinceTiltChange >= TiltChangeInterval)
		{
			TimeSinceTiltChange = 0.0f;
			TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
			TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
		}

		// tilt interp
		float TiltOsc = TiltOscAmplitude * FMath::Sin(2.0f * PI * TiltOscFrequency * TimeSinceTiltChange);
		TiltAngleCurrent = FMath::FInterpTo(TiltAngleCurrent, TiltAngleTarget + TiltOsc, DeltaTime, TiltLerpSpeed);

		// phase update
		ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
		ChaseCurvePhase = FMath::Fmod(ChaseCurvePhase, 2.0f * PI);

		// desired radius inside attack range (e.g. 0.9 * AttackR)
		float RadiusOsc = AttackR * 0.15f * FMath::Sin(ChaseCurvePhase * 0.5f + 0.3f);
		float MaxOsc = AttackR * 0.2f;
		RadiusOsc = FMath::Clamp(RadiusOsc, -MaxOsc, MaxOsc);
		float DesiredRadius = FMath::Clamp(AttackR * 0.9f + RadiusOsc, OrbitMin, OrbitMax);

		// basis
		FVector Axis = RotationAxis.IsNearlyZero() ? FVector::UpVector : RotationAxis.GetSafeNormal();
		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f) ? FVector::RightVector : FVector::UpVector;
		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		FVector Radial = (Right * FMath::Cos(ChaseCurvePhase) + Forward * FMath::Sin(ChaseCurvePhase)).GetSafeNormal();
		FVector TiltAxis = FRotator(0.0f, TiltAxisYaw, 0.0f).RotateVector(Right).GetSafeNormal();
		FQuat TiltQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));
		FVector OrbitDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

		FVector OrbitTargetPos = TargetLoc + OrbitDir * DesiredRadius;

		// move toward OrbitTargetPos smoothly (max step)
		FVector ToOrbit = OrbitTargetPos - CurrentLoc;
		float DistToOrbit = ToOrbit.Size();
		if (DistToOrbit > KINDA_SMALL_NUMBER)
		{
			FVector MoveDir = ToOrbit / DistToOrbit;
			float Speed = FMath::Max(1.0f, _spawnedInfo.Move_Speed);
			float MaxStep = Speed * DeltaTime;
			FVector MoveDelta = (DistToOrbit > MaxStep) ? MoveDir * MaxStep : ToOrbit;
			AddActorWorldOffset(MoveDelta, true);
		}

		return;
	}

	// Fallback: 기본 접근
	{
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _spawnedInfo.Move_Speed);
		AddActorWorldOffset(Dir * Speed * DeltaTime, true);
	}
}

void ADroneEnemy::GoToTarget(FVector CurrentLoc, FVector TargetLoc, FVector ApproachPoint, float DeltaTime)
{
	float Dist = FVector::Dist(CurrentLoc, TargetLoc);

	// 공전 진입 거리: 사거리의 0.7배
	float OrbitEnterDist = _spawnedInfo.Attack_Range * 0.7f;

	// 공전 탈출 거리: 사거리의 1.5배
	float OrbitExitDist = _spawnedInfo.Attack_Range * 1.5f;

	// 공전 진입
	if (!bOrbiting && Dist <= OrbitEnterDist)
	{
		EnterOrbit();
		return;
	}

	// 공전 탈출
	if (bOrbiting && Dist >= OrbitExitDist)
	{
		bOrbiting = false;
		return;
	}

	// 그냥 ApproachPoint로 부드럽게 이동 (최대 스텝 제한)
	FVector ToGoal = ApproachPoint - CurrentLoc;
	float DistToGoal = ToGoal.Size();
	if (DistToGoal > KINDA_SMALL_NUMBER)
	{
		FVector Dir = ToGoal / DistToGoal;
		float Speed = FMath::Max(1.0f, _spawnedInfo.Move_Speed);
		float MaxStep = Speed * DeltaTime;
		FVector Move = (DistToGoal > MaxStep) ? Dir * MaxStep : ToGoal;
		AddActorWorldOffset(Move, true);
	}
}


void ADroneEnemy::EnterOrbit()
{
	bOrbiting = true;

	float DegPerSec = FMath::FRandRange(10.0f, 25.0f);
	float Sign = FMath::RandBool() ? 1.0f : -1.0f;

	AngularSpeedTarget = FMath::DegreesToRadians(DegPerSec) * Sign;

	// 현재 속도가 0에 가까우면 바로 목표로 시작
	if (FMath::IsNearlyZero(AngularSpeedCurrent))
		AngularSpeedCurrent = AngularSpeedTarget;

	TimeSinceDirChange = 0.0f;

	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);

	TimeSinceTiltChange = 0.0f;
}

void ADroneEnemy::OrbitAroundTarget(const FVector& ApproachPoint, float DeltaTime)
{
	if (!Target) return;

	FVector TargetLoc = Target->GetActorLocation();
	FVector CurrentLoc = GetActorLocation();
	float CurrentDist = FVector::Dist(CurrentLoc, TargetLoc);

	// 공전 중이면서 아직 공격 사거리 내가 아니면 계속 접근
	if (bOrbiting && CurrentDist > _spawnedInfo.Attack_Range * 0.8f)
	{
		// 접근 단계: 목표 거리(사거리의 75%)로 천천히 접근
		float TargetDist = _spawnedInfo.Attack_Range * 0.75f;
		FVector DirToTarget = (TargetLoc - CurrentLoc).GetSafeNormal();

		float Speed = _spawnedInfo.Move_Speed * 0.6f; // 공전 중 접근 속도 (느리게)
		if (Speed <= 0.0f) Speed = 60.0f;

		FVector MoveDir = DirToTarget;
		float MaxStep = Speed * DeltaTime;

		AddActorWorldOffset(MoveDir * MaxStep, true);
		return;
	}

	// 공전 상태 유지 로직
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
			AngularSpeedTarget *= -1.0f;
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
		FMath::Sin(GetWorld()->TimeSeconds * 2 * PI * TiltOscFrequency);

	float FinalTiltAngle = TiltAngleCurrent + TiltOsc;

	FVector OrbitAxis =
		FRotator(
			FinalTiltAngle,
			TiltAxisYaw,
			0.0f
		).RotateVector(FVector::UpVector);

	FVector Offset = GetActorLocation() - ApproachPoint;

	float AngleDeg =
		FMath::RadiansToDegrees(AngularSpeedCurrent * DeltaTime);

	FVector RotatedOffset =
		Offset.RotateAngleAxis(AngleDeg, OrbitAxis);

	SetActorLocation(ApproachPoint + RotatedOffset, true);
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

void ADroneEnemy::CheckChaseDistance()
{
	// 일정 거리 이내면 추격 시작 (기존 DistanceCheck 사용)
	if (DistanceCheck(_spawnedInfo.Detection_Range))
	{
		LoseTargetTime = 0.0f;

		if (!bIsChasing)
			ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);

		bIsChasing = true;
		Target = _spaceShip;
	}
	else
	{
		LoseTargetTime += GetWorld()->DeltaTimeSeconds;

		if (LoseTargetTime >= LoseTargetDelay)
		{
			bIsChasing = false;
			bOrbiting = false;
			Target = nullptr;
		}
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