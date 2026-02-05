#include "CJH/Enemy/DroneEnemy.h"

#include "CJH/Enemy/Weapon/Bullet.h"
#include "JHS/GameControl/JHSGameMode.h"

#include "Net/UnrealNetwork.h"

ADroneEnemy::ADroneEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	// 일정한 Tick 간격 설정 (매프레임 대신 일정 간격으로)
	PrimaryActorTick.TickInterval = 0.016f;  // ~60Hz 고정

	// 드론 본체 메쉬 (루트)
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	RootComponent = TurretMesh;

	// 총구 위치 표시용 Arrow
	MuzzleArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	MuzzleArrow->SetupAttachment(RootComponent);

	// 기본 회전 축 (Z축 기준 공전)
	RotationAxis = FVector(0.0f, 0.0f, 1.0f);

	// 공전 관련 초기값
	ChaseCurveAmplitude = 300.0f;
	ChaseCurveFrequency = 0.5f;
	ChaseCurvePhase = 0.0f;
	ChaseCurveSign = 1.0f;

	// 각속도 초기값
	AngularSpeedCurrent = FMath::DegreesToRadians(15.0f);
	AngularSpeedTarget = AngularSpeedCurrent;

	// Tilt 관련 초기값
	TiltAngleCurrent = 0.0f;
	TiltAngleTarget = 0.0f;
	TiltLerpSpeed = 0.5f;
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	TimeSinceTiltChange = 0.0f;
	TiltOscAmplitude = 5.0f;
	TiltOscFrequency = 0.2f;

	// ===== 극단 최적화 =====
	bReplicates = true;
	NetUpdateFrequency = 8.0f;      // 15Hz → 8Hz (네트워크 50% 감소)
	MinNetUpdateFrequency = 4.0f;   // 최소 4Hz
	SetReplicateMovement(false);    // 위치/회전 리플리케이션 비활성화
}

void ADroneEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	// ===== MuzzleArrow 위치 동기화 =====
	if (MuzzleArrow && TurretMesh)
	{
		MuzzleArrow->SetRelativeLocation(FVector::ZeroVector);
	}

	ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);
	ChaseCurveSign = FMath::FRand() > 0.5f ? 1.0f : -1.0f;

	TimeSinceDirChange = 0.0f;

	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
	TimeSinceTiltChange = 0.0f;

	// ===== LOD 타이머 시작: 거리 기반 최적화 =====
	GetWorld()->GetTimerManager().SetTimer(
		LODTimerHandle,
		this,
		&ADroneEnemy::UpdateLOD,
		0.5f,  // 0.5초마다 거리 체크
		true
	);
}

void ADroneEnemy::Tick(float DeltaTime)
{
	// ===== 클라이언트: 서버 상태만 보간 =====
	if (!HasAuthority())
	{
		InterpAlpha += DeltaTime * 15.0f;

		FTransform NewTransform = FTransform::Identity;
		NewTransform.Blend(
			PrevTransform,
			ServerTransform,
			FMath::Clamp(InterpAlpha, 0.0f, 1.0f)
		);

		SetActorTransform(NewTransform, false);
		return;
	}

	// ===== 서버: 실제 로직 실행 =====
	Super::Tick(DeltaTime);

	// ===== LOD에 따라 처리 빈도 조절 =====
	static int32 TickCounter = 0;
	TickCounter++;

	int32 TickSkipInterval = 1;  // 기본값: 모든 Tick 실행
	
	if (CurrentLOD == ELODLevel::VeryFar)
		TickSkipInterval = 4;  // 4프레임마다 한 번
	else if (CurrentLOD == ELODLevel::Far)
		TickSkipInterval = 2;  // 2프레임마다 한 번
	// ELODLevel::Close면 모든 프레임 실행

	if (TickCounter % TickSkipInterval != 0)
		return;

	if (CanCheck)
		CheckChaseDistance();

	if (!bIsChasing)
		bOrbiting = false;

	if (bIsChasing && Target && IsValid(_spaceShip))
	{
		ChaseMove(DeltaTime);

		if (DistanceCheck(_spawnedInfo.Attack_Range))
			Fire();
	}
	else
	{
		FollowOrbitTarget(DeltaTime);
		ApplySpin(DeltaTime);
	}

	if (bIsChasing && Target && IsValid(_spaceShip))
	{
		LookTarget();
	}

	ServerTransform = GetActorTransform();
}

// ===== 거리 기반 LOD 업데이트 =====
void ADroneEnemy::UpdateLOD()
{
	if (!HasAuthority() || !_spaceShip)
		return;

	float Distance = FVector::Dist(GetActorLocation(), _spaceShip->GetActorLocation());

	ELODLevel NewLOD = ELODLevel::Close;

	// ===== 거리에 따라 LOD 결정 (1 UU = 1 cm 기준) =====
	// Detection Range가 10,000 UU(100m)이므로 그보다 큰 범위로 설정
	if (Distance > 50000.0f)  // 500m 이상
		NewLOD = ELODLevel::VeryFar;
	else if (Distance > 25000.0f)  // 250m 이상
		NewLOD = ELODLevel::Far;
	else
		NewLOD = ELODLevel::Close;  // 250m 이하

	// LOD 변경 시에만 업데이트
	if (NewLOD != CurrentLOD)
	{
		CurrentLOD = NewLOD;
		ApplyLODSettings();
	}
}

// ===== LOD 설정 적용 =====
void ADroneEnemy::ApplyLODSettings()
{
	switch (CurrentLOD)
	{
	case ELODLevel::Close:
		// 최고 품질: 모든 기능 활성화 (0~250m)
		NetUpdateFrequency = 8.0f;
		if (TurretMesh)
			TurretMesh->SetVisibility(true);
		break;

	case ELODLevel::Far:
		// 중간 품질: 네트워크 업데이트 감소 (250m~500m)
		NetUpdateFrequency = 4.0f;
		if (TurretMesh)
			TurretMesh->SetVisibility(true);
		break;

	case ELODLevel::VeryFar:
		// 낮은 품질: 최소 네트워크 업데이트 (500m 이상)
		NetUpdateFrequency = 2.0f;
		if (TurretMesh)
			TurretMesh->SetVisibility(false);  // 렌더링 중단
		break;
	}
}

void ADroneEnemy::ChaseMove(float DeltaTime)
{
	if (!IsValid(Target))
		return;

	const FVector TargetLoc = Target->GetActorLocation();
	FVector CurrentLoc = GetActorLocation();

	const float AttackR = _spawnedInfo.Attack_Range;
	const float OrbitMin = AttackR * 0.7f;
	const float OrbitMax = AttackR;

	float CurrentDistToTarget = FVector::Dist(CurrentLoc, TargetLoc);

	// 1) 너무 가까우면 뒤로 물러남
	if (CurrentDistToTarget < OrbitMin)
	{
		FVector BackDir = (CurrentLoc - TargetLoc).GetSafeNormal();
		if (BackDir.IsNearlyZero())
			BackDir = FVector::ForwardVector;

		FVector BackTarget = TargetLoc + BackDir * OrbitMin;
		FVector ToBack = BackTarget - CurrentLoc;
		float Dist = ToBack.Size();

		if (Dist > KINDA_SMALL_NUMBER)
		{
			FVector Dir = ToBack / Dist;
			float Speed = FMath::Max(1.0f, _targetInfo.Speed);
			float MaxStep = Speed * DeltaTime;
			FVector Move = (Dist > MaxStep) ? Dir * MaxStep : ToBack;
			AddActorWorldOffset(Move, false);  // 콜리전 체크 제거
		}

		bOrbiting = false;
		return;
	}

	// 2) 공격 사거리 밖이면 직진
	if (CurrentDistToTarget > AttackR)
	{
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _targetInfo.Speed);
		AddActorWorldOffset(Dir * Speed * DeltaTime, false);

		bOrbiting = false;
		return;
	}

	// 3) 공전 구간 (비용이 높으므로 이미 최적화된 상태 유지)
	if (CurrentDistToTarget >= OrbitMin && CurrentDistToTarget <= OrbitMax)
	{
		if (!bOrbiting)
			EnterOrbit();

		// 각속도 보간
		AngularSpeedCurrent = FMath::FInterpTo(
			AngularSpeedCurrent,
			AngularSpeedTarget,
			DeltaTime,
			AngularLerpSpeed
		);

		// 방향 변경 주기
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

		// Tilt 변경
		TimeSinceTiltChange += DeltaTime;
		if (TimeSinceTiltChange >= TiltChangeInterval)
		{
			TimeSinceTiltChange = 0.0f;
			TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
			TiltAxisYaw = FMath::RandRange(0.0f, 360.0f);
		}

		// Tilt 보간
		float TiltOsc = TiltOscAmplitude * FMath::Sin(2.0f * PI * TiltOscFrequency * TimeSinceTiltChange);
		TiltAngleCurrent = FMath::FInterpTo(TiltAngleCurrent, TiltAngleTarget + TiltOsc, DeltaTime, TiltLerpSpeed);

		// 위상 업데이트
		ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
		ChaseCurvePhase = FMath::Fmod(ChaseCurvePhase, 2.0f * PI);

		// 궤도 반경 계산
		float RadiusOsc = AttackR * 0.15f * FMath::Sin(ChaseCurvePhase * 0.5f + 0.3f);
		float MaxOsc = AttackR * 0.2f;
		RadiusOsc = FMath::Clamp(RadiusOsc, -MaxOsc, MaxOsc);
		float DesiredRadius = FMath::Clamp(AttackR * 0.9f + RadiusOsc, OrbitMin, OrbitMax);

		// Orbit 기저 벡터 계산
		FVector Axis = RotationAxis.IsNearlyZero() ? FVector::UpVector : RotationAxis.GetSafeNormal();
		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f) ? FVector::RightVector : FVector::UpVector;
		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		FVector Radial = (Right * FMath::Cos(ChaseCurvePhase) + Forward * FMath::Sin(ChaseCurvePhase)).GetSafeNormal();
		FVector TiltAxis = FRotator(0.0f, TiltAxisYaw, 0.0f).RotateVector(Right).GetSafeNormal();
		FQuat TiltQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));
		FVector OrbitDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

		FVector OrbitTargetPos = TargetLoc + OrbitDir * DesiredRadius;

		// Orbit 방향으로 이동
		FVector ToOrbit = OrbitTargetPos - CurrentLoc;
		float DistToOrbit = ToOrbit.Size();

		if (DistToOrbit > KINDA_SMALL_NUMBER)
		{
			FVector MoveDir = ToOrbit / DistToOrbit;
			float Speed = FMath::Max(1.0f, _targetInfo.Speed);
			float MaxStep = Speed * DeltaTime;
			FVector MoveDelta = (DistToOrbit > MaxStep) ? MoveDir * MaxStep : ToOrbit;
			AddActorWorldOffset(MoveDelta, false);  // 콜리전 체크 제거
		}

		return;
	}

	// Fallback: 기본 접근
	{
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _targetInfo.Speed);
		AddActorWorldOffset(Dir * Speed * DeltaTime, false);
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
		float Speed = FMath::Max(1.0f, _targetInfo.Speed);
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
	if (!Target)
		return;

	FVector TargetLoc = Target->GetActorLocation();
	FVector CurrentLoc = GetActorLocation();
	float CurrentDist = FVector::Dist(CurrentLoc, TargetLoc);

	// 공전 중이면서 아직 공격 사거리 내가 아니면 계속 접근
	if (bOrbiting && CurrentDist > _spawnedInfo.Attack_Range * 0.8f)
	{
		// 접근 단계: 목표 거리(사거리의 75%)로 천천히 접근
		float TargetDist = _spawnedInfo.Attack_Range * 0.75f;
		FVector DirToTarget = (TargetLoc - CurrentLoc).GetSafeNormal();

		float Speed = _targetInfo.Speed * 0.6f; // 공전 중 접근 속도 (느리게)

		if (Speed <= 0.0f) 
			Speed = 60.0f;

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

void ADroneEnemy::LookTarget()
{
	if (!IsValid(Target))
		return;

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	
	// 거리가 거의 0이면 조기 종료
	if (ToTarget.IsNearlyZero(1.0f))
		return;

	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = ToTarget.Rotation();

	// 각도 차이 계산
	float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);
	float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentRot.Pitch, TargetRot.Pitch);

	// 데드존으로 떨림 방지 (0.5도)
	const float DeadAngle = 0.5f;
	if (FMath::Abs(DeltaYaw) < DeadAngle && FMath::Abs(DeltaPitch) < DeadAngle)
		return;

	// 부드러운 회전
	FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		TargetRot,
		GetWorld()->GetDeltaSeconds(),
		2.5f
	);

	SetActorRotation(NewRot);
}
void ADroneEnemy::Fire()
{
	if (!HasAuthority())
		return;
	if (!CanFire)
		return;
	if (!Bullet)
		return;

	// ===== MuzzleArrow 위치 검증: 0,0,0 파티클 방지 =====
	if (!MuzzleArrow)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire: MuzzleArrow is null"));
		return;
	}

	FVector MuzzleLocation = MuzzleArrow->GetComponentLocation();
	
	// 0,0,0 근처인지 확인
	if (MuzzleLocation.IsNearlyZero(50.f))
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire: MuzzleArrow at origin (0,0,0)"));
		return;
	}

	FRotator MuzzleRotation = MuzzleArrow->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	// ===== 총알 스폰 먼저 =====
	ABullet* SpawnedBullet = GetWorld()->SpawnActor<ABullet>(
		Bullet,
		MuzzleLocation,
		MuzzleRotation,
		SpawnParams
	);

	if (SpawnedBullet)
	{
		SpawnedBullet->SetOwnerActor(this);
		SpawnedBullet->Fire(GetActorForwardVector());
		SpawnedBullet->SetDamage(_targetInfo.Attack_Damage);
	}

	// ===== 그 다음 이펙트 (서버에서만 멀티캐스트) =====
	MulticastFireEffect();

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

void ADroneEnemy::MulticastFireEffect_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
		return;

	// ===== 클라이언트도 MuzzleArrow 유효성 검증 =====
	if (!FireParticle || !MuzzleArrow)
		return;

	FVector MuzzleLocation = MuzzleArrow->GetComponentLocation();
	
	// 0,0,0 근처면 스킵
	if (MuzzleLocation.IsNearlyZero(50.f))
		return;

	UGameplayStatics::SpawnEmitterAttached(
		FireParticle,
		MuzzleArrow,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTargetIncludingScale
	);
}

void ADroneEnemy::CheckChaseDistance()
{
	CanCheck = false;


	if (bIsChasing && !TargetHPCheck())
	{
		bIsChasing = false;
		bOrbiting = false;
		Target = nullptr;
		_spaceShip = nullptr;

		return;
	}

	// 일정 거리 이내면 추격 시작 (기존 DistanceCheck 사용)
	if (IsValid(_spaceShip) && DistanceCheck(_spawnedInfo.Detection_Range))
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

	GetWorld()->GetTimerManager().SetTimer(
		CanDistanceHandle,
		this,
		&ADroneEnemy::CanCheckDistance,
		1.f,
		false
	);
}


void ADroneEnemy::ApplySpin(float DeltaTime)
{
	if (RotationAxis.IsNearlyZero())
		return;

	FRotator CurrentRotation = GetActorRotation();
	FVector CurrentAxis = CurrentRotation.RotateVector(RotationAxis);

	FQuat SpinQuat = FQuat(CurrentAxis, FMath::DegreesToRadians(SpinSpeed * DeltaTime));
	FQuat NewQuat = SpinQuat * GetActorQuat();

	SetActorRotation(NewQuat);  
}

void ADroneEnemy::OnRep_ServerTransform()
{
	PrevTransform = GetActorTransform();
	InterpAlpha = 0.0f;
}

void ADroneEnemy::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADroneEnemy, ServerTransform);
}
