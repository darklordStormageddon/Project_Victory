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
	PrimaryComponentTick.TickInterval = 0.2f;  // 10Hz → 5Hz (50% 감소)

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

		// ===== 극단 최적화: 오비트 타이머 간격 확대 =====
		GetWorld()->GetTimerManager().SetTimer(
			OrbitTimerHandle,
			this,
			&UGarbageEnemyManagerComponent::TurnOrbit,
			0.1f,  // 0.05초 → 0.1초 (20Hz → 10Hz, 50% 감소)
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

			FVector Center = GetCenterLocation();
			FRotator R = FRotator(OrbitPitch, 0.0f, 0.0f);
			FVector Dir = R.Vector().GetSafeNormal();
			FVector Target = Center + Dir * OrbitDistance;
			JuniorEnemies.Add(Garbage, Target);

			SetDroneProperties(Garbage);
		}

		if (GarbageEnemies.Num() == 1)
		{
			OrbitSet();
			BuildOrbitStructure();
		}
		else
		{
			BuildOrbitStructure();
		}
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

	AngleStep = 360.0f / static_cast<float>(GarbageCount);

	TiltQuat = FQuat(OrbitTiltAxis.GetSafeNormal(), FMath::DegreesToRadians(OrbitPitch));
}


void UGarbageEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ===== 클라이언트: Tick 완전 스킵 =====
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
}

// ===== 타이머 기반 오비트 계산 =====
void UGarbageEnemyManagerComponent::TurnOrbit()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const float FixedDeltaTime = 0.1f;
	
	CurrentOrbitPhase += RotateSpeed * FixedDeltaTime;
	if (CurrentOrbitPhase >= 360.0f) 
		CurrentOrbitPhase -= 360.0f;

	FVector Center = GetCenterLocation();
	
	int32 GarbageCount = GarbageEnemies.Num();
	if (GarbageCount == 0)
		return;

	// ===== 극단 최적화: 매 3프레임마다만 업데이트 =====
	static int32 UpdateCounter = 0;
	UpdateCounter++;
	
	int32 UpdateInterval = 3;  // 10Hz → 3Hz 실제 업데이트
	if (UpdateCounter % UpdateInterval != 0)
		return;

	for (int32 i = 0; i < GarbageCount; ++i)
	{
		AGarbageEnemyBase* GarbageEnemy = GarbageEnemies[i];
		if (!GarbageEnemy || !IsValid(GarbageEnemy)) 
			continue;

		ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(GarbageEnemy);
		if (DroneEnemy && DroneEnemy->IsChasing())
			continue;

		float Yaw = i * AngleStep + CurrentOrbitPhase;
		float YawRad = FMath::DegreesToRadians(Yaw);

		float CosYaw = FMath::Cos(YawRad);
		float SinYaw = FMath::Sin(YawRad);
		FVector Radial(CosYaw, SinYaw, 0.0f);

		FVector TiltedDir = TiltQuat.RotateVector(Radial).GetSafeNormal();
		FVector TargetPos = Center + TiltedDir * OrbitDistance;

		JuniorEnemies[GarbageEnemy] = TargetPos;

		GarbageEnemy->SetOrbitTarget(TargetPos);
	}
}

void UGarbageEnemyManagerComponent::DebugVector()
{
	if (!GetOwner()->HasAuthority())
		return;

	for (auto& elem : JuniorEnemies)
	{
		DrawDebugSphere(
			GetWorld(),
			elem.Value,
			_debugRadius,
			16,
			FColor::Red,
			false,
			0.1f
		);
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