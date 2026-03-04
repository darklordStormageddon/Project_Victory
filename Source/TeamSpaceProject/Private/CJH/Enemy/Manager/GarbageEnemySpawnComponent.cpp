// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Enemy/Manager/GarbageEnemySpawnComponent.h"

#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "CJH/Enemy/DroneEnemy.h"

#include "DrawDebugHelpers.h"

UGarbageEnemySpawnComponent::UGarbageEnemySpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;

	OrbitMinDistance = 800.0f;
	OrbitMaxDistance = 1800.0f;
	OrbitDistance = 1500.0f;
	OrbitPitchRange = 45.0f;
	RotateMinSpeed = 30.0f;
	RotateMaxSpeed = 70.0f;
	_debugRadius = 100.0f;
}

void UGarbageEnemySpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			OrbitTimerHandle,
			this,
			&UGarbageEnemySpawnComponent::TurnOrbit,
			0.05f,
			true
		);
	}
}

void UGarbageEnemySpawnComponent::SetDroneProperties(AEnemyBase* Drone)
{
	if (!Drone) return;

	ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(Drone);
	if (DroneEnemy)
	{
		FVector RandomAxis = FMath::VRand();
		DroneEnemy->SetSpinAxis(RandomAxis);
	}
}

void UGarbageEnemySpawnComponent::InitializeOrbitData(AGarbageEnemyBase* Drone)
{
	if (!Drone)
		return;

	if (OrbitData.Contains(Drone))
		return;

	FDroneOrbitData Data;
	Data.OrbitRadius = FMath::RandRange(OrbitMinDistance, OrbitMaxDistance);

	FVector Axis = FMath::VRand();
	if (Axis.IsNearlyZero())
		Axis = FVector::UpVector;

	Data.OrbitAxis = Axis.GetSafeNormal();
	Data.Phase = FMath::RandRange(0.0f, 360.0f);
	Data.AngularSpeed = FMath::RandRange(RotateMinSpeed, RotateMaxSpeed);

	OrbitData.Add(Drone, Data);
}

void UGarbageEnemySpawnComponent::OrbitSet()
{
	OrbitDistance = FMath::RandRange(OrbitMinDistance, OrbitMaxDistance);
	OrbitPitch = FMath::RandRange(-OrbitPitchRange, OrbitPitchRange);
	RotateSpeed = FMath::RandRange(RotateMinSpeed, RotateMaxSpeed);

	float RandomYawForTilt = FMath::RandRange(0.0f, 360.0f);
	OrbitTiltAxis = FRotator(0.0f, RandomYawForTilt, 0.0f).Vector().GetSafeNormal();

	CurrentOrbitPhase = FMath::RandRange(0.0f, 360.0f);
}

void UGarbageEnemySpawnComponent::BuildOrbitStructure()
{
	int32 GarbageCount = GarbageEnemies.Num();
	
	if (GarbageCount == 0)
		return;

	for (AGarbageEnemyBase* Enemy : GarbageEnemies)
	{
		if (!Enemy)
			continue;

		InitializeOrbitData(Enemy);
	}

	AngleStep = 360.0f / static_cast<float>(GarbageCount);

	TiltQuat = FQuat(OrbitTiltAxis.GetSafeNormal(), FMath::DegreesToRadians(OrbitPitch));
}


void UGarbageEnemySpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (_debug)
		DebugVector();
}

void UGarbageEnemySpawnComponent::TurnOrbit()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const float FixedDeltaTime = 0.05f;

	CurrentOrbitPhase += RotateSpeed * FixedDeltaTime;
	if (CurrentOrbitPhase >= 360.0f)
		CurrentOrbitPhase -= 360.0f;

	FVector Center = GetCenterLocation();

	int32 GarbageCount = GarbageEnemies.Num();
	if (GarbageCount == 0)
		return;

	for (int32 i = 0; i < GarbageCount; ++i)
	{
		AGarbageEnemyBase* GarbageEnemy = GarbageEnemies[i];
		if (!GarbageEnemy || !IsValid(GarbageEnemy))
			continue;

		ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(GarbageEnemy);
		if (DroneEnemy)
			continue; // 드론은 자체 OrbitMoveServer로 궤도를 계산하므로 외부 위치 지정 불필요

		FDroneOrbitData* Data = OrbitData.Find(GarbageEnemy);
		if (!Data)
		{
			InitializeOrbitData(GarbageEnemy);
			Data = OrbitData.Find(GarbageEnemy);
		}

		if (!Data)
			continue;

		Data->Phase += Data->AngularSpeed * FixedDeltaTime;
		if (Data->Phase >= 360.0f)
			Data->Phase -= 360.0f;

		FVector Axis = Data->OrbitAxis;
		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
			? FVector::RightVector : FVector::UpVector;

		FVector Right   = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		float Rad    = FMath::DegreesToRadians(Data->Phase);
		FVector Radial = Right * FMath::Cos(Rad) + Forward * FMath::Sin(Rad);

		FVector TargetPos = Center + Radial * Data->OrbitRadius;

		JuniorEnemies[GarbageEnemy] = TargetPos;

		// 접선 속도 = ω(rad/s) * r
		const float AngularRad  = FMath::DegreesToRadians(Data->AngularSpeed);
		const float TangentSpeed = AngularRad * Data->OrbitRadius;

		GarbageEnemy->SetOrbitPosition(TargetPos, TangentSpeed);
	}
}

void UGarbageEnemySpawnComponent::OnEnemySpawned(AEnemyBase* SpawnedEnemyBase)
{
	AGarbageEnemyBase* Garbage = Cast<AGarbageEnemyBase>(SpawnedEnemyBase);
	if (!Garbage)
		return;

	Garbage->EnemyComponent = this;
	GarbageEnemies.Add(Garbage);

	Garbage->SetTargetShip(_spaceShip);
	Garbage->OwnerGET(_owner);
	Garbage->ComponentGET(this);

	SetDroneProperties(Garbage);
	InitializeOrbitData(Garbage);

	const FDroneOrbitData* Data = OrbitData.Find(Garbage);
	if (Data)
	{
		FVector Center = GetCenterLocation();
		FVector Axis = Data->OrbitAxis;

		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
			? FVector::RightVector
			: FVector::UpVector;

		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		float Rad = FMath::DegreesToRadians(Data->Phase);
		FVector Radial = Right * FMath::Cos(Rad) + Forward * FMath::Sin(Rad);

		FVector Target = Center + Radial * Data->OrbitRadius;
		JuniorEnemies.Add(Garbage, Target);
		Garbage->SetOrbitTarget(Target);
	}
}

void UGarbageEnemySpawnComponent::DebugVector()
{
	if (!GetOwner()->HasAuthority())
		return;

	FVector Center = GetCenterLocation();

	for (auto& elem : JuniorEnemies)
	{
		AGarbageEnemyBase* GarbageEnemy = elem.Key;
		const FVector& Target = elem.Value;

		const FDroneOrbitData* Data = OrbitData.Find(GarbageEnemy);
		if (!Data)
			continue;

		FVector Axis = Data->OrbitAxis;
		FVector Temp = (FMath::Abs(FVector::DotProduct(Axis, FVector::UpVector)) > 0.99f)
			? FVector::RightVector
			: FVector::UpVector;

		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		if (_debugOrbitLine)
		{
			DrawDebugCircle(
				GetWorld(),
				Center,
				Data->OrbitRadius,
				64,
				FColor::Green,
				false,
				0.1f,
				0,
				2.0f,
				Right,
				Forward,
				false
			);
		}

		if (_debugOrbitPoint)
		{
			DrawDebugSphere(
				GetWorld(),
				Target,
				_debugRadius,
				12,
				FColor::Red,
				false,
				0.1f
			);
		}
	}
}

void UGarbageEnemySpawnComponent::RemoveEnemies(AEnemyBase* _removeEnemy)
{
	Super::RemoveEnemies(_removeEnemy);
	
	if (!GetOwner() || !GetOwner()->HasAuthority() || !_removeEnemy)
		return;

	AGarbageEnemyBase* GarbageEnemy = Cast<AGarbageEnemyBase>(_removeEnemy);

	GarbageEnemies.Remove(GarbageEnemy);
	GarbageEnemies.Shrink();

	JuniorEnemies.Remove(GarbageEnemy);
	OrbitData.Remove(GarbageEnemy);
	
	BuildOrbitStructure();
}

void UGarbageEnemySpawnComponent::DeleteAllEnemy()
{
	Super::DeleteAllEnemy();

	GetWorld()->GetTimerManager().ClearTimer(OrbitTimerHandle);

	TArray<AEnemyBase*> EnemiesToClear;
	for (AGarbageEnemyBase* Enemy : GarbageEnemies)
	{
		EnemiesToClear.Add(Enemy);
	}

	GarbageEnemies.Empty();
	JuniorEnemies.Empty();
	OrbitData.Empty();

	for (AEnemyBase* SpawnEnemy : EnemiesToClear)
	{
		if (!IsValid(SpawnEnemy))
			continue;

		SpawnEnemy->Destroy();
	}
}
