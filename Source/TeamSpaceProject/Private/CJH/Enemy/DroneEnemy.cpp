#include "CJH/Enemy/DroneEnemy.h"
#include "CJH/Enemy/Weapon/Bullet.h"
#include "Net/UnrealNetwork.h"

ADroneEnemy::ADroneEnemy()
{
	// 클라이언트 보간은 매 프레임 실행해야 부드러움 - TickInterval 제거
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = TurretMesh;

	MuzzleArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	MuzzleArrow->SetupAttachment(RootComponent);

	bReplicates = true;
	SetReplicateMovement(false);
	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 10.0f;
	NetPriority = 2.0f;
	SetNetDormancy(DORM_Never);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::BeginPlay()
{
	Super::BeginPlay();

	ClientSmoothLoc  = GetActorLocation();
	ClientSmoothRot  = GetActorRotation();
	ClientTargetLoc  = GetActorLocation();
	ClientTargetRot  = GetActorRotation();
	bDroneClientInit = false;

	if (!HasAuthority())
		return;

	ChaseCurvePhase = FMath::RandRange(0.f, 2.f * PI);
	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw     = FMath::RandRange(0.f, 360.f);

	// 타겟 감지가면 타이머로 (0.5초마다)
	GetWorld()->GetTimerManager().SetTimer(
		CanDistanceHandle, this, &ADroneEnemy::CheckTarget, 0.5f, true);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::OnRep_DroneState()
{
	const FVector NewLoc = FVector(RepDroneLoc);

	if (!bDroneClientInit)
	{
		ClientSmoothLoc  = NewLoc;
		ClientTargetLoc  = NewLoc;
		ClientSmoothRot  = RepRotation;
		ClientTargetRot  = RepRotation;
		bDroneClientInit = true;
		SetActorLocationAndRotation(NewLoc, RepRotation, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	const float Dist = FVector::Dist(ClientSmoothLoc, NewLoc);
	if (Dist > SnapDistance)
	{
		ClientSmoothLoc = NewLoc;
		ClientTargetLoc = NewLoc;
	}
	else
	{
		ClientTargetLoc = NewLoc;
	}
}

void ADroneEnemy::OnRep_DroneRot()
{
	if (!bDroneClientInit)
		return;

	ClientTargetRot = RepRotation;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::Tick(float DeltaTime)
{
	// GarbageEnemyBase::Tick을 통해 Super 체인 유지
	// ClientTickInterp은 override해서 부모 보간 차단
	AGarbageEnemyBase::Tick(DeltaTime);

	if (HasAuthority())
	{
		ServerMove(DeltaTime);
		return;
	}

	if (!bDroneClientInit)
		return;

	// ── 공전 중: 클라이언트가 독립적으로 궤도 계산 ─────────────
	// 네트워크 업데이트(20Hz)와 무관하게 매 프레임 부드럽게 이동
	if (!bIsChasing && IsValid(_owner))
	{
		// 서버 각도가 처음 도착하면 동기화
		if (!bClientOrbitSynced)
		{
			ClientOrbitAngle  = RepOrbitAngle;
			bClientOrbitSynced = true;
		}

		const FVector Axis  = RotationAxis.IsNearlyZero()
			? FVector::UpVector : RotationAxis.GetSafeNormal();
		const FVector Temp  = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
			? FVector::RightVector : FVector::UpVector;
		const FVector Right   = FVector::CrossProduct(Temp,  Axis).GetSafeNormal();
		const FVector Forward = FVector::CrossProduct(Axis,  Right).GetSafeNormal();

		// 서버와 동일한 속도로 각도 누적
		const float OrbitSpeed = FMath::DegreesToRadians(30.0f);
		ClientOrbitAngle += OrbitSpeed * DeltaTime;
		if (ClientOrbitAngle > 2.0f * PI)
			ClientOrbitAngle -= 2.0f * PI;

		// 서버 각도와 오차 보정 (너무 벌어지면 부드럽게 동기화)
		// FMath::FindDeltaAngle은 없으므로 라디안 델타 직접 계산
		float AngleDiff = RepOrbitAngle - ClientOrbitAngle;
		// -PI ~ PI 범위로 정규화
		while (AngleDiff >  PI) AngleDiff -= 2.0f * PI;
		while (AngleDiff < -PI) AngleDiff += 2.0f * PI;
		ClientOrbitAngle += AngleDiff * FMath::Min(DeltaTime * 2.0f, 1.0f);

		const float   OrbitR   = 800.0f;
		const FVector OwnerLoc = _owner->GetActorLocation();
		const FVector OrbitPos = OwnerLoc
			+ Right   * OrbitR * FMath::Cos(ClientOrbitAngle)
			+ Forward * OrbitR * FMath::Sin(ClientOrbitAngle);

		// 자전
		if (!RotationAxis.IsNearlyZero())
		{
			const FQuat SpinQ(
				GetActorQuat().RotateVector(RotationAxis).GetSafeNormal(),
				FMath::DegreesToRadians(SpinSpeed * DeltaTime));
			ClientSmoothRot = (SpinQ * GetActorQuat()).Rotator();
		}

		SetActorLocationAndRotation(OrbitPos, ClientSmoothRot, false, nullptr, ETeleportType::None);
		ClientSmoothLoc = OrbitPos;
		return;
	}

	// ── 추격 중: VInterpTo 보간 ─────────────────────────────────
	bClientOrbitSynced = false; // 추격 끝나면 각도 재동기화

	ClientSmoothLoc = FMath::VInterpTo(ClientSmoothLoc, ClientTargetLoc, DeltaTime, ClientInterpSpeed);
	ClientSmoothRot = FMath::RInterpTo(ClientSmoothRot, ClientTargetRot, DeltaTime, ClientInterpSpeed);

	SetActorLocationAndRotation(
		ClientSmoothLoc, ClientSmoothRot,
		false, nullptr, ETeleportType::None);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::ServerMove(float DeltaTime)
{
	if (!HasAuthority())
		return;

	if (bIsChasing && IsValid(Target))
	{
		// LookAtTarget을 이동 전에 호출 → 현재 위치 기준으로 방향 계산
		// 이동 후 호출하면 매 프레임 방향이 미세하게 달라져 버벅임 발생
		LookAtTarget(DeltaTime);
		ChaseMoveServer(DeltaTime);

		if (DistanceCheck(_spawnedInfo.Attack_Range))
			TryFire();
	}
	else
	{
		OrbitMoveServer(DeltaTime);
	}

	// GarbageEnemyBase의 RepLocation이 아닌 드론 전용 변수에 저장
	RepDroneLoc = GetActorLocation();
	RepRotation = GetActorRotation();
}

// ─────────────────────────────────────────────────────────────────────────────
// 추격 이동: 공격 범위 이내에서 랜덤 궤도로 movimientos
void ADroneEnemy::ChaseMoveServer(float DeltaTime)
{
	const FVector TargetLoc  = Target->GetActorLocation();
	const FVector CurrentLoc = GetActorLocation();
	const float   AttackR    = _spawnedInfo.Attack_Range;
	const float   Speed      = FMath::Max(_targetInfo.Speed, 1.0f);

	const float DistSq = FVector::DistSquared(CurrentLoc, TargetLoc);
	const float OrbitR = AttackR * 0.85f;

	// 너무 가까우면 뒤로
	if (DistSq < (OrbitR * 0.7f) * (OrbitR * 0.7f))
	{
		const FVector Away = (CurrentLoc - TargetLoc).GetSafeNormal();
		AddActorWorldOffset(Away * Speed * DeltaTime, false);
		return;
	}

	// 너무 멀면 접근
	if (DistSq > AttackR * AttackR)
	{
		const FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		AddActorWorldOffset(Dir * Speed * DeltaTime, false);
		return;
	}

	// 공격 범위 내 → 랜덤 궤도 이동
	AngularSpeedCurrent = FMath::FInterpTo(
		AngularSpeedCurrent, AngularSpeedTarget, DeltaTime, AngularLerpSpeed);

	TimeSinceDirChange += DeltaTime;
	if (TimeSinceDirChange >= DirChangeInterval)
	{
		TimeSinceDirChange = 0.0f;
		if (FMath::FRand() < DirReverseProbability)
			AngularSpeedTarget *= -1.0f;
		else
		{
			const float DegSec = FMath::FRandRange(8.0f, 25.0f);
			AngularSpeedTarget = FMath::DegreesToRadians(DegSec) * (FMath::RandBool() ? 1.f : -1.f);
		}
	}

	TimeSinceTiltChange += DeltaTime;
	if (TimeSinceTiltChange >= TiltChangeInterval)
	{
		TimeSinceTiltChange = 0.0f;
		TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
		TiltAxisYaw     = FMath::RandRange(0.0f, 360.0f);
	}

	TiltAngleCurrent = FMath::FInterpTo(
		TiltAngleCurrent, TiltAngleTarget, DeltaTime, TiltLerpSpeed);

	ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
	ChaseCurvePhase  = FMath::Fmod(ChaseCurvePhase, 2.0f * PI);

	// 궤도 위 목표 위치 계산
	const FVector Axis = RotationAxis.IsNearlyZero()
		? FVector::UpVector : RotationAxis.GetSafeNormal();
	const FVector Temp  = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
		? FVector::RightVector : FVector::UpVector;
	const FVector Right   = FVector::CrossProduct(Temp,  Axis).GetSafeNormal();
	const FVector Forward = FVector::CrossProduct(Axis,  Right).GetSafeNormal();

	const FVector Radial    = (Right * FMath::Cos(ChaseCurvePhase) + Forward * FMath::Sin(ChaseCurvePhase)).GetSafeNormal();
	const FVector TiltAxis  = FRotator(0.f, TiltAxisYaw, 0.f).RotateVector(Right).GetSafeNormal();
	const FQuat   TiltQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));
	const FVector OrbitDir  = TiltQuat.RotateVector(Radial).GetSafeNormal();

	const FVector OrbitGoal  = TargetLoc + OrbitDir * OrbitR;
	const FVector ToGoal     = OrbitGoal - CurrentLoc;
	const float   ToGoalDist = ToGoal.Size();

	if (ToGoalDist > KINDA_SMALL_NUMBER)
	{
		const float Step = Speed * DeltaTime;
		AddActorWorldOffset(ToGoal / ToGoalDist * FMath::Min(Step, ToGoalDist), false);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 궤도 공전: GarbageEnemyBase의 FollowOrbitTarget은 그대로 활용
void ADroneEnemy::OrbitMoveServer(float DeltaTime)
{
	// ── 자전 공통 처리 ─────────────────────────────────────────
	auto DoSpin = [&]()
	{
		if (!RotationAxis.IsNearlyZero())
		{
			const FQuat SpinQ(
				GetActorQuat().RotateVector(RotationAxis).GetSafeNormal(),
				FMath::DegreesToRadians(SpinSpeed * DeltaTime));
			SetActorRotation(SpinQ * GetActorQuat());
		}
	};

	if (!IsValid(_owner))
	{
		DoSpin();
		return;
	}

	const FVector OwnerLoc = _owner->GetActorLocation();
	const FVector Axis  = RotationAxis.IsNearlyZero()
		? FVector::UpVector : RotationAxis.GetSafeNormal();
	const FVector Temp  = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
		? FVector::RightVector : FVector::UpVector;
	const FVector Right   = FVector::CrossProduct(Temp,  Axis).GetSafeNormal();
	const FVector Forward = FVector::CrossProduct(Axis,  Right).GetSafeNormal();

	const float OrbitSpeed = FMath::DegreesToRadians(30.0f);
	OrbitAngle += OrbitSpeed * DeltaTime;
	if (OrbitAngle > 2.0f * PI)
		OrbitAngle -= 2.0f * PI;

	// RepOrbitAngle도 동기화 (클라이언트 예측 보정용)
	RepOrbitAngle = OrbitAngle;

	const float   OrbitR   = 800.0f;
	const FVector OrbitPos = OwnerLoc
		+ Right   * OrbitR * FMath::Cos(OrbitAngle)
		+ Forward * OrbitR * FMath::Sin(OrbitAngle);

	SetActorLocation(OrbitPos, false, nullptr, ETeleportType::None);
	DoSpin();
}

// ─────────────────────────────────────────────────────────────────────────────
// 타겟 방향 회전 (서버에서만, 결과는 RepRotation으로 복제)
void ADroneEnemy::LookAtTarget(float DeltaTime)
{
	if (!IsValid(Target))
		return;

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (ToTarget.IsNearlyZero(1.0f))
		return;

	const FRotator WantRot    = ToTarget.Rotation();
	const FRotator CurrentRot = GetActorRotation();

	// RInterpConstantTo 대신 RInterpTo(부드러운 감속) 사용
	// 초당 10 = 빠르게 목표 방향으로 수렴, 완전히 도달하면 더 이상 변화 없음
	SetActorRotation(FMath::RInterpTo(CurrentRot, WantRot, DeltaTime, 10.0f));
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::TryFire()
{
	if (!CanFire || !Bullet || !MuzzleArrow)
		return;

	const FVector MuzzleLoc = MuzzleArrow->GetComponentLocation();
	if (MuzzleLoc.IsNearlyZero(50.f))
		return;

	// 발사 방향: 타겟 직접 조준
	FVector FireDir = FVector::ForwardVector;
	if (IsValid(Target))
		FireDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();

	FActorSpawnParameters Params;
	Params.Owner      = this;
	Params.Instigator = GetInstigator();

	ABullet* Shot = GetWorld()->SpawnActor<ABullet>(
		Bullet, MuzzleLoc, FireDir.Rotation(), Params);

	if (Shot)
	{
		Shot->SetOwnerActor(this);
		Shot->Fire(FireDir);
		Shot->SetDamage(_targetInfo.Attack_Damage);
	}

	Multicast_FireEffect();
	CanFire = false;

	GetWorld()->GetTimerManager().SetTimer(
		FireCooldownHandle, this, &ADroneEnemy::EnableFiring,
		FMath::Max(_spawnedInfo.Attack_Speed, 0.1f), false);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::Multicast_FireEffect_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer || !FireParticle || !MuzzleArrow)
		return;

	const FVector Loc = MuzzleArrow->GetComponentLocation();
	if (Loc.IsNearlyZero(50.f))
		return;

	UGameplayStatics::SpawnEmitterAttached(
		FireParticle, MuzzleArrow, NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTargetIncludingScale);
}

// ─────────────────────────────────────────────────────────────────────────────
// 0.5초마다 타이머로 호출 - 감지 범위·타겟 유효성 체크
void ADroneEnemy::CheckTarget()
{
	if (!HasAuthority())
		return;

	// 타겟이 죽었으면 추격 해제
	if (bIsChasing && !TargetHPCheck())
	{
		bIsChasing = false;
		Target     = nullptr;
		LoseTargetTimer = 0.0f;
		return;
	}

	if (IsValid(_spaceShip) && DistanceCheck(_spawnedInfo.Detection_Range))
	{
		if (!bIsChasing)
			ChaseCurvePhase = FMath::RandRange(0.0f, 2.0f * PI);

		bIsChasing      = true;
		Target          = _spaceShip;
		LoseTargetTimer = 0.0f;
	}
	else
	{
		LoseTargetTimer += 0.5f;
		if (LoseTargetTimer >= LoseTargetDelay)
		{
			bIsChasing = false;
			Target     = nullptr;
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ADroneEnemy, RepDroneLoc, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ADroneEnemy, RepRotation, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ADroneEnemy, bIsChasing);
	DOREPLIFETIME(ADroneEnemy, RepOrbitAngle);
}
