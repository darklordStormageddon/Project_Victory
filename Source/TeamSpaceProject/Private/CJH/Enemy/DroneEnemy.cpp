#include "CJH/Enemy/DroneEnemy.h"

#include "CJH/Enemy/Weapon/Bullet.h"
#include "JHS/GameControl/JHSGameMode.h"

#include "Net/UnrealNetwork.h"

ADroneEnemy::ADroneEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = TurretMesh;

	MuzzleArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	MuzzleArrow->SetupAttachment(RootComponent);

	RotationAxis = FVector(0.f, 0.f, 1.f);

	bReplicates = true;
	SetReplicateMovement(false);

	NetUpdateFrequency = 20.f;
	MinNetUpdateFrequency = 10.f;
	NetPriority = 2.0f;

	bAlwaysRelevant = false;
	NetCullDistanceSquared = 400000000.f;
	SetNetDormancy(DORM_Never);
}

void ADroneEnemy::BeginPlay()
{
	Super::BeginPlay();

	RepLocation = GetActorLocation();
	RepRotation = GetActorRotation();
	RepVelocity = FVector::ZeroVector;

	if (!HasAuthority())
	{
		ClientTargetLoc = GetActorLocation();
		ClientTargetRot = GetActorRotation();
		ClientVelocity = FVector::ZeroVector;
		return;
	}

	ChaseCurvePhase = FMath::RandRange(0.f, 2.f * PI);
	ChaseCurveSign = FMath::FRand() > 0.5f ? 1.f : -1.f;

	TimeSinceDirChange = 0.f;

	TiltAngleTarget = FMath::RandRange(-TiltAngleRange, TiltAngleRange);
	TiltAxisYaw = FMath::RandRange(0.f, 360.f);
	TimeSinceTiltChange = 0.f;
}

void ADroneEnemy::OnRep_ServerState()
{
	const FVector NewServerLoc = FVector(RepLocation);
	const FVector NewVelocity = FVector(RepVelocity);

	// 속도 업데이트
	ClientVelocity = NewVelocity;
	ClientTargetRot = RepRotation;

	if (!bClientInitialized)
	{
		ClientTargetLoc = NewServerLoc;
		bClientInitialized = true;
		return;
	}

	// 서버 위치와 예측 타겟의 차이 → 절반만 보정 (떨림 방지)
	const FVector Error = NewServerLoc - ClientTargetLoc;
	ClientTargetLoc += Error * 0.5f;
}

void ADroneEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		const FVector LocBefore = GetActorLocation();

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
			LookTarget();

		const FVector LocAfter = GetActorLocation();
		if (DeltaTime > KINDA_SMALL_NUMBER)
			RepVelocity = (LocAfter - LocBefore) / DeltaTime;
		else
			RepVelocity = FVector::ZeroVector;

		RepLocation = LocAfter;
		RepRotation = GetActorRotation();
	}
	else
	{
		if (!bClientInitialized)
			return;

		// 속도로 예측 타겟 이동
		ClientTargetLoc += ClientVelocity * DeltaTime;

		// 액터를 타겟 위치로 직접 이동 (VInterpTo 제거 → 떨림 원인 제거)
		const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), ClientTargetRot, DeltaTime, 10.0f);
		SetActorLocationAndRotation(ClientTargetLoc, NewRot, false, nullptr, ETeleportType::TeleportPhysics);
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

	const float OrbitMinSq = OrbitMin * OrbitMin;
	const float OrbitMaxSq = OrbitMax * OrbitMax;
	const float AttackRSq = AttackR * AttackR;

	const float CurrentDistSqToTarget = FVector::DistSquared(CurrentLoc, TargetLoc);

	if (CurrentDistSqToTarget < OrbitMinSq)
	{
		FVector BackDir = (CurrentLoc - TargetLoc).GetSafeNormal();
		if (BackDir.IsNearlyZero())
			BackDir = FVector::ForwardVector;

		FVector BackTarget = TargetLoc + BackDir * OrbitMin;
		FVector ToBack = BackTarget - CurrentLoc;
		float DistSq = ToBack.SizeSquared();

		if (DistSq > KINDA_SMALL_NUMBER)
		{
			float Dist = FMath::Sqrt(DistSq);
			FVector Dir = ToBack / Dist;
			float Speed = FMath::Max(1.0f, _targetInfo.Speed);
			float MaxStep = Speed * DeltaTime;
			FVector Move = (Dist > MaxStep) ? Dir * MaxStep : ToBack;
			AddActorWorldOffset(Move, false);
		}

		bOrbiting = false;
		return;
	}

	if (CurrentDistSqToTarget > AttackRSq)
	{
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _targetInfo.Speed);
		AddActorWorldOffset(Dir * Speed * DeltaTime, false);

		bOrbiting = false;
		return;
	}

	if (CurrentDistSqToTarget >= OrbitMinSq && CurrentDistSqToTarget <= OrbitMaxSq)
	{
		if (!bOrbiting)
			EnterOrbit();

		AngularSpeedCurrent = FMath::FInterpTo(AngularSpeedCurrent, AngularSpeedTarget, DeltaTime, AngularLerpSpeed);

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

		float TiltOsc = TiltOscAmplitude * FMath::Sin(2.0f * PI * TiltOscFrequency * TimeSinceTiltChange);
		TiltAngleCurrent = FMath::FInterpTo(TiltAngleCurrent, TiltAngleTarget + TiltOsc, DeltaTime, TiltLerpSpeed);

		ChaseCurvePhase += AngularSpeedCurrent * DeltaTime;
		ChaseCurvePhase = FMath::Fmod(ChaseCurvePhase, 2.0f * PI);

		float RadiusOsc = AttackR * 0.15f * FMath::Sin(ChaseCurvePhase * 0.5f + 0.3f);
		float MaxOsc = AttackR * 0.2f;
		RadiusOsc = FMath::Clamp(RadiusOsc, -MaxOsc, MaxOsc);
		float DesiredRadius = FMath::Clamp(AttackR * 0.9f + RadiusOsc, OrbitMin, OrbitMax);

		FVector Axis = RotationAxis.IsNearlyZero() ? FVector::UpVector : RotationAxis.GetSafeNormal();
		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f) ? FVector::RightVector : FVector::UpVector;
		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		FVector Radial = (Right * FMath::Cos(ChaseCurvePhase) + Forward * FMath::Sin(ChaseCurvePhase)).GetSafeNormal();
		FVector TiltAxis = FRotator(0.0f, TiltAxisYaw, 0.0f).RotateVector(Right).GetSafeNormal();
		FQuat TiltQuat(TiltAxis, FMath::DegreesToRadians(TiltAngleCurrent));
		FVector OrbitDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

		FVector OrbitTargetPos = TargetLoc + OrbitDir * DesiredRadius;

		FVector ToOrbit = OrbitTargetPos - CurrentLoc;
		float DistToOrbitSq = ToOrbit.SizeSquared();

		if (DistToOrbitSq > KINDA_SMALL_NUMBER)
		{
			float DistToOrbit = FMath::Sqrt(DistToOrbitSq);
			FVector MoveDir = ToOrbit / DistToOrbit;
			float Speed = FMath::Max(1.0f, _targetInfo.Speed);
			float MaxStep = Speed * DeltaTime;
			FVector MoveDelta = (DistToOrbit > MaxStep) ? MoveDir * MaxStep : ToOrbit;
			AddActorWorldOffset(MoveDelta, false);
		}

		return;
	}

	{
		FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
		float Speed = FMath::Max(1.0f, _targetInfo.Speed);
		AddActorWorldOffset(Dir * Speed * DeltaTime, false);
	}
}

void ADroneEnemy::GoToTarget(FVector CurrentLoc, FVector TargetLoc, FVector ApproachPoint, float DeltaTime)
{
	float DistSq = FVector::DistSquared(CurrentLoc, TargetLoc);

	float OrbitEnterDist = _spawnedInfo.Attack_Range * 0.7f;
	float OrbitExitDist = _spawnedInfo.Attack_Range * 1.5f;
	float OrbitEnterDistSq = OrbitEnterDist * OrbitEnterDist;
	float OrbitExitDistSq = OrbitExitDist * OrbitExitDist;

	if (!bOrbiting && DistSq <= OrbitEnterDistSq)
	{
		EnterOrbit();
		return;
	}

	if (bOrbiting && DistSq >= OrbitExitDistSq)
	{
		bOrbiting = false;
		return;
	}

	FVector ToGoal = ApproachPoint - CurrentLoc;
	float DistToGoalSq = ToGoal.SizeSquared();
	if (DistToGoalSq > KINDA_SMALL_NUMBER)
	{
		float DistToGoal = FMath::Sqrt(DistToGoalSq);
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
	float CurrentDistSq = FVector::DistSquared(CurrentLoc, TargetLoc);

	float OrbitExitThreshold = _spawnedInfo.Attack_Range * 0.8f;
	float OrbitExitThresholdSq = OrbitExitThreshold * OrbitExitThreshold;

	if (bOrbiting && CurrentDistSq > OrbitExitThresholdSq)
	{
		float TargetDist = _spawnedInfo.Attack_Range * 0.75f;
		FVector DirToTarget = (TargetLoc - CurrentLoc).GetSafeNormal();

		float Speed = _targetInfo.Speed * 0.6f;

		if (Speed <= 0.0f) 
			Speed = 60.0f;

		FVector MoveDir = DirToTarget;
		float MaxStep = Speed * DeltaTime;

		AddActorWorldOffset(MoveDir * MaxStep, true);
		return;
	}

	AngularSpeedCurrent = FMath::FInterpTo(AngularSpeedCurrent, AngularSpeedTarget, DeltaTime, AngularLerpSpeed);

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

	TiltAngleCurrent = FMath::FInterpTo(TiltAngleCurrent, TiltAngleTarget, DeltaTime, TiltLerpSpeed);

	float TiltOsc = TiltOscAmplitude * FMath::Sin(GetWorld()->TimeSeconds * 2 * PI * TiltOscFrequency);
	float FinalTiltAngle = TiltAngleCurrent + TiltOsc;

	FVector OrbitAxis = FRotator(FinalTiltAngle, TiltAxisYaw, 0.0f).RotateVector(FVector::UpVector);
	FVector Offset = GetActorLocation() - ApproachPoint;
	float AngleDeg = FMath::RadiansToDegrees(AngularSpeedCurrent * DeltaTime);
	FVector RotatedOffset = Offset.RotateAngleAxis(AngleDeg, OrbitAxis);

	SetActorLocation(ApproachPoint + RotatedOffset, true);
}

void ADroneEnemy::LookTarget()
{
	if (!IsValid(Target))
		return;

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	
	if (ToTarget.IsNearlyZero(1.0f))
		return;

	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = ToTarget.Rotation(); 

	float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);
	float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentRot.Pitch, TargetRot.Pitch);

	const float DeadAngle = 0.5f;
	if (FMath::Abs(DeltaYaw) < DeadAngle && FMath::Abs(DeltaPitch) < DeadAngle)
		return;

	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), 2.5f);
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

	if (!MuzzleArrow)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire: MuzzleArrow is null"));
		return;
	}

	FVector MuzzleLocation = MuzzleArrow->GetComponentLocation();
	
	if (MuzzleLocation.IsNearlyZero(50.f))
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire: MuzzleArrow at origin (0,0,0)"));
		return;
	}

	FRotator MuzzleRotation = MuzzleArrow->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	ABullet* SpawnedBullet = GetWorld()->SpawnActor<ABullet>(Bullet, MuzzleLocation, MuzzleRotation, SpawnParams);

	if (SpawnedBullet)
	{
		SpawnedBullet->SetOwnerActor(this);
		SpawnedBullet->Fire(GetActorForwardVector());
		SpawnedBullet->SetDamage(_targetInfo.Attack_Damage);
	}

	MulticastFireEffect();

	CanFire = false;

	FTimerHandle FireRateHandle;
	GetWorld()->GetTimerManager().SetTimer(FireRateHandle, this, &ADroneEnemy::EnableFiring, _spawnedInfo.Attack_Speed, false);
}

void ADroneEnemy::MulticastFireEffect_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
		return;

	if (!FireParticle || !MuzzleArrow)
		return;

	FVector MuzzleLocation = MuzzleArrow->GetComponentLocation();
	
	if (MuzzleLocation.IsNearlyZero(50.f))
		return;

	UGameplayStatics::SpawnEmitterAttached(
		FireParticle, MuzzleArrow, NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator,
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

	GetWorld()->GetTimerManager().SetTimer(CanDistanceHandle, this, &ADroneEnemy::CanCheckDistance, 1.f, false);
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

void ADroneEnemy::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ADroneEnemy, RepLocation, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ADroneEnemy, RepRotation);
	DOREPLIFETIME(ADroneEnemy, RepVelocity);
	DOREPLIFETIME(ADroneEnemy, bIsChasing);
	DOREPLIFETIME(ADroneEnemy, ChaseCurvePhase);
	DOREPLIFETIME(ADroneEnemy, bOrbiting);
	DOREPLIFETIME(ADroneEnemy, AngularSpeedCurrent);
	DOREPLIFETIME(ADroneEnemy, TiltAngleCurrent);
	DOREPLIFETIME(ADroneEnemy, TiltAxisYaw);
	DOREPLIFETIME(ADroneEnemy, ServerLocation);
	DOREPLIFETIME(ADroneEnemy, ServerRotation);
}
