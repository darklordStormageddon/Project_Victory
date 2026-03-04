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

	// 서버 이동은 0.05초 타이머로 분리 → Tick 부하 없음
	GetWorld()->GetTimerManager().SetTimer(
		ServerMoveTimerHandle,
		[this]() { if (IsValid(this)) ServerMove(ServerMoveDeltaTime); },
		ServerMoveDeltaTime, true);

	// 타겟 감지는 0.5초마다
	GetWorld()->GetTimerManager().SetTimer(
		CanDistanceHandle, this, &ADroneEnemy::CheckTarget, 0.5f, true);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::OnRep_DroneState()
{
	const FVector  NewLoc = FVector(RepLocation);
	const FRotator NewRot = RepRotation;

	if (!bDroneClientInit)
	{
		ClientSmoothLoc   = NewLoc;
		ClientTargetLoc   = NewLoc;
		ClientPrevLoc     = NewLoc;
		ClientInterpAlpha = 1.0f;
		ClientInterpSpeed = 0.0f;
		ClientSmoothRot   = NewRot;
		ClientTargetRot   = NewRot;
		ClientPrevRot     = NewRot;
		ClientRotAlpha    = 1.0f;
		bDroneClientInit  = true;
		return;
	}

	const float Dist = FVector::Dist(ClientSmoothLoc, NewLoc);

	if (Dist > SnapDistance)
	{
		// 너무 멀면 즉시 스냅
		ClientSmoothLoc   = NewLoc;
		ClientTargetLoc   = NewLoc;
		ClientPrevLoc     = NewLoc;
		ClientInterpAlpha = 1.0f;
		ClientInterpSpeed = 0.0f;
	}
	else
	{
		ClientPrevLoc     = ClientSmoothLoc;
		ClientTargetLoc   = NewLoc;
		ClientInterpAlpha = 0.0f;
		ClientInterpSpeed = (Dist > KINDA_SMALL_NUMBER) ? (Dist / 0.05f) : 0.0f;
	}

	ClientPrevRot   = ClientSmoothRot;
	ClientTargetRot = NewRot;
	ClientRotAlpha  = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADroneEnemy::Tick(float DeltaTime)
{
	AActor::Tick(DeltaTime);

	if (HasAuthority())
		return;

	if (!bDroneClientInit)
		return;

	// 위치: Alpha를 DeltaTime/0.05씩 증가 → 항상 0.05초에 선형 이동
	if (ClientInterpAlpha < 1.0f)
	{
		ClientInterpAlpha += DeltaTime / 0.05f;
		ClientInterpAlpha  = FMath::Min(ClientInterpAlpha, 1.0f);
		ClientSmoothLoc    = FMath::Lerp(ClientPrevLoc, ClientTargetLoc, ClientInterpAlpha);
	}
	else
	{
		ClientSmoothLoc = ClientTargetLoc;
	}

	// 회전: 동일하게 0.05초에 선형 보간
	if (ClientRotAlpha < 1.0f)
	{
		ClientRotAlpha += DeltaTime / 0.05f;
		ClientRotAlpha  = FMath::Min(ClientRotAlpha, 1.0f);
		ClientSmoothRot = FMath::Lerp(ClientPrevRot, ClientTargetRot, ClientRotAlpha);
	}
	else
	{
		ClientSmoothRot = ClientTargetRot;
	}

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
		ChaseMoveServer(DeltaTime);
		LookAtTarget();

		if (DistanceCheck(_spawnedInfo.Attack_Range))
			TryFire();
	}
	else
	{
		OrbitMoveServer(DeltaTime);
	}

	// 이동 후 복제 변수 갱신
	RepLocation = GetActorLocation();
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
// 궤도 공전: GarbageEnemyBase의 FollowOrbitTarget을 그대로 활용
void ADroneEnemy::OrbitMoveServer(float DeltaTime)
{
	FollowOrbitTarget(DeltaTime);

	// 자전
	if (!RotationAxis.IsNearlyZero())
	{
		const FQuat SpinQ(
			GetActorQuat().RotateVector(RotationAxis).GetSafeNormal(),
			FMath::DegreesToRadians(SpinSpeed * DeltaTime));
		SetActorRotation(SpinQ * GetActorQuat());
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 타겟 방향 회전 (서버에서만, 결과는 RepRotation으로 복제)
void ADroneEnemy::LookAtTarget()
{
	if (!IsValid(Target))
		return;

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (ToTarget.IsNearlyZero(1.0f))
		return;

	// 속도 기반 고속 회전 (틱 간격 0.05초이므로 빠르게)
	const FRotator WantRot = ToTarget.Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), WantRot, 0.05f, 10.0f));
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

	// RepLocation은 부모(AGarbageEnemyBase)에서 등록
	DOREPLIFETIME_CONDITION_NOTIFY(ADroneEnemy, RepRotation, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ADroneEnemy, bIsChasing);
}
