// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Enemy/Manager/GarbageEnemyManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "CJH/Enemy/Base/SpawnedEnemyBase.h"
#include "CJH/Enemy/DroneEnemy.h"

#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGarbageEnemyManagerComponent::UGarbageEnemyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// ===== 극단 최적화: Tick 간격 확대 =====
	PrimaryComponentTick.TickInterval = 0.05f;  // 10Hz → 5Hz (50% 감소)

	OrbitMinDistance = 800.0f;
	OrbitMaxDistance = 1800.0f;
	OrbitDistance = 1500.0f;
	OrbitPitchRange = 45.0f;
	RotateMinSpeed = 30.0f;
	RotateMaxSpeed = 70.0f;
	_debugRadius = 100.0f;
}

// Called when the game starts
void UGarbageEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		GarbageSpawnSetting();

		GetWorld()->GetTimerManager().SetTimer(
			OrbitTimerHandle,
			this,
			&UGarbageEnemyManagerComponent::TurnOrbit,
			0.05f,
			true
		);
	}
}

void UGarbageEnemyManagerComponent::GarbageSpawnSetting()
{
	FVector CenterLocation;

	if (_owner)
		CenterLocation = _owner->GetActorLocation();
	else if (GetOwner())
		CenterLocation = GetOwner()->GetActorLocation();
	else
		CenterLocation = FVector::ZeroVector;

	FRotator SpawnRotation = FRotator::ZeroRotator;

	float _spawnNum = FMath::RandRange(_minSpawn, _maxSpawn);

	for (int i = 0; i < _spawnNum; i++)
		SpawnInMap(CenterLocation, SpawnRotation, _Enemy);
}

void UGarbageEnemyManagerComponent::SpawnInMap(FVector SpawnLocation, FRotator SpawnRotation, TArray<TSubclassOf<AGarbageEnemyBase>> _spawn_enemy)
{
	if (_spawn_enemy.Num() == 0)
		return;

	float SpawnEnemyNum = FMath::RandRange(0, _spawn_enemy.Num() - 1);

	SpawnEnemy(_spawn_enemy[SpawnEnemyNum], SpawnLocation, SpawnRotation);
}

void UGarbageEnemyManagerComponent::SpawnEnemy(
	TSubclassOf<AGarbageEnemyBase> Enemy,
	FVector SpawnLocation,
	FRotator SpawnRotator)
{
	SpawnedEnemy = GetWorld()->SpawnActor<AGarbageEnemyBase>(Enemy, SpawnLocation, SpawnRotator);

	if (SpawnedEnemy)
	{
		AGarbageEnemyBase* Garbage = Cast<AGarbageEnemyBase>(SpawnedEnemy);
		if (Garbage)
		{
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

		BuildOrbitStructure();
	}
}

void UGarbageEnemyManagerComponent::SetDroneProperties(AEnemyBase* Drone)
{
	if (!Drone) return;

	ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(Drone);
	if (DroneEnemy)
	{
		FVector RandomAxis = FMath::VRand();
		DroneEnemy->SetSpinAxis(RandomAxis);
	}
}

void UGarbageEnemyManagerComponent::InitializeOrbitData(AGarbageEnemyBase* Drone)
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

void UGarbageEnemyManagerComponent::OrbitSet()
{
	OrbitDistance = FMath::RandRange(OrbitMinDistance, OrbitMaxDistance);
	OrbitPitch = FMath::RandRange(-OrbitPitchRange, OrbitPitchRange);
	RotateSpeed = FMath::RandRange(RotateMinSpeed, RotateMaxSpeed);

	float RandomYawForTilt = FMath::RandRange(0.0f, 360.0f);
	OrbitTiltAxis = FRotator(0.0f, RandomYawForTilt, 0.0f).Vector().GetSafeNormal();

	CurrentOrbitPhase = FMath::RandRange(0.0f, 360.0f);
}

void UGarbageEnemyManagerComponent::BuildOrbitStructure()
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


void UGarbageEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ===== 클라이언트: Tick 완전 스킵 =====
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (_debug)
		DebugVector();
}

// ===== 타이머 기반 오비트 계산 =====
void UGarbageEnemyManagerComponent::TurnOrbit()
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
		if (DroneEnemy && DroneEnemy->IsChasing())
			continue;

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
			? FVector::RightVector
			: FVector::UpVector;

		FVector Right = FVector::CrossProduct(Temp, Axis).GetSafeNormal();
		FVector Forward = FVector::CrossProduct(Axis, Right).GetSafeNormal();

		float Rad = FMath::DegreesToRadians(Data->Phase);
		FVector Radial = Right * FMath::Cos(Rad) + Forward * FMath::Sin(Rad);

		FVector TargetPos = Center + Radial * Data->OrbitRadius;

		JuniorEnemies[GarbageEnemy] = TargetPos;

		GarbageEnemy->SetOrbitTarget(TargetPos);
	}
}

void UGarbageEnemyManagerComponent::DebugVector()
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

void UGarbageEnemyManagerComponent::RemoveEnemies(AEnemyBase* _removeEnemy)
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

void UGarbageEnemyManagerComponent::DeleteAllEnemy()
{
	Super::DeleteAllEnemy();

	GetWorld()->GetTimerManager().ClearTimer(OrbitTimerHandle);

	for (AEnemyBase* SpawnEnemy : GarbageEnemies)
	{
		if (!IsValid(SpawnEnemy))
			continue;

		SpawnEnemy->Destroy();
	}

	JuniorEnemies.Empty();
	JuniorEnemies.Shrink();

	GarbageEnemies.Empty();
	GarbageEnemies.Shrink();
}