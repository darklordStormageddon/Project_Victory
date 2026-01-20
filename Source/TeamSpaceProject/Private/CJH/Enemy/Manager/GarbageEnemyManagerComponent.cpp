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

	// 기본값(필요시 Detail에서 조정)
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

	GarbageSpawnSetting();
}

void UGarbageEnemyManagerComponent::GarbageSpawnSetting()
{
	FVector CenterLocation;

	if (_owner)
		CenterLocation = _owner->GetActorLocation();
	else
		CenterLocation = GetOwner()->GetActorLocation();

	FVector RandomDirection = FMath::VRand();

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

			// 초기 target 자리 채우기 (MakeOrbitStructure가 다음 Tick에서 덮어씀)
			FVector Center = GetCenterLocation();
			FRotator R = FRotator(OrbitPitch, 0.0f, 0.0f); // 모든 자식이 공유하는 피치 사용 (초기자리)
			FVector Dir = R.Vector().GetSafeNormal();
			FVector Target = Center + Dir * OrbitDistance;
			JuniorEnemies.Add(Garbage, Target);

			// 드론 속성 설정(자전축 등 기존 로직 유지)
			SetDroneProperties(Garbage);
		}

		OrbitSet();

		BuildOrbitStructure();
	}
}

void UGarbageEnemyManagerComponent::SetDroneProperties(AEnemyBase* Drone)
{
	if (!Drone) return;

	if (Drone->IsA<ADroneEnemy>())
	{
		ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(Drone);
		if (DroneEnemy)
		{
			FVector RandomAxis = FMath::VRand();
			DroneEnemy->SetSpinAxis(RandomAxis);
		}
	}
}

void UGarbageEnemyManagerComponent::OrbitSet()
{
	// 모든 Junior가 공유하는 공전 파라미터 결정(컴포넌트 단위)
	OrbitDistance = FMath::RandRange(OrbitMinDistance, OrbitMaxDistance);
	OrbitPitch = FMath::RandRange(-OrbitPitchRange, OrbitPitchRange);
	RotateSpeed = FMath::RandRange(RotateMinSpeed, RotateMaxSpeed);

	// 궤도 평면 기울일 축을 수평 방향에서 랜덤으로 선택 (XY 평면의 단위벡터)
	float RandomYawForTilt = FMath::RandRange(0.0f, 360.0f);
	OrbitTiltAxis = FRotator(0.0f, RandomYawForTilt, 0.0f).Vector().GetSafeNormal();

	// 그룹 위상 초기화(랜덤 시작 가능)
	CurrentOrbitPhase = FMath::RandRange(0.0f, 360.0f);
}

void UGarbageEnemyManagerComponent::BuildOrbitStructure()
{
	Num = GarbageEnemies.Num();
	if (Num == 0)
		return;

	// 각 엔티티에 균등하게 각도 분배 (컴포넌트 단위로 동일한 OrbitPitch/OrbitDistance 사용)
	AngleStep = 360.0f / float(Num);

	// 쿼터니언 하나로 궤도 평면 전체를 회전시킬 준비
	TiltQuat = FQuat(OrbitTiltAxis.GetSafeNormal(), FMath::DegreesToRadians(OrbitPitch));
}


void UGarbageEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (candebug && _debug)
		DebugVector();

	TurnOrbit(DeltaTime);
}

void UGarbageEnemyManagerComponent::TurnOrbit(float DeltaTime)
{
	// 그룹 위상(도) 증가
	CurrentOrbitPhase += RotateSpeed * DeltaTime;
	if (CurrentOrbitPhase >= 360.0f) CurrentOrbitPhase = FMath::Fmod(CurrentOrbitPhase, 360.0f);
	if (CurrentOrbitPhase < 0.0f) CurrentOrbitPhase += 360.0f;

	// 중심 위치(garbage의 오너 혹은 컴포넌트 소유자)
	FVector Center = GetCenterLocation();

	for (int i = 0; i < Num; ++i)
	{
		AGarbageEnemyBase* GarbageEnemy = GarbageEnemies[i];
		if (!GarbageEnemy) continue;

		// === 추격 중인지 확인 (드론인 경우) ===
		ADroneEnemy* DroneEnemy = Cast<ADroneEnemy>(GarbageEnemy);
		if (DroneEnemy && DroneEnemy->IsChasing())
			continue;

		// 그룹 위상(CurrentOrbitPhase)을 더해 전체가 회전하도록 함
		float Yaw = i * AngleStep + CurrentOrbitPhase;
		float YawRad = FMath::DegreesToRadians(Yaw);

		// 원형의 기본 단위벡터 (XY 평면)
		FVector Radial = FVector(FMath::Cos(YawRad), FMath::Sin(YawRad), 0.0f).GetSafeNormal();

		// 전체 궤도 평면을 TiltQuat로 회전 -> 동일한 tilt 축/각도로 모든 점을 이동시킴
		FVector TiltedDir = TiltQuat.RotateVector(Radial).GetSafeNormal();

		FVector TargetPos = Center + TiltedDir * OrbitDistance;

		// 맵 갱신 및 엔티티에 목표 전달
		JuniorEnemies[GarbageEnemy] = TargetPos;

		// 엔티티(가비지)에 목표 전달 - 엔티티는 이 목표를 따라가기만 하면 됨
		GarbageEnemy->SetOrbitTarget(TargetPos);
	}
}

void UGarbageEnemyManagerComponent::DebugVector()
{
	candebug = false;

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

	FTimerHandle debugdelay;

	GetWorld()->GetTimerManager().SetTimer(debugdelay, this, &UGarbageEnemyManagerComponent::CanDebug, 0.1f, false);
  
}



void UGarbageEnemyManagerComponent::RemoveEnemies(AEnemyBase* _removeEnemy)
{
	Super::RemoveEnemies(_removeEnemy);
	
	if (_removeEnemy)
	{
		AGarbageEnemyBase* GarbageEnemy = Cast<AGarbageEnemyBase>(_removeEnemy);

		GarbageEnemies.Remove(GarbageEnemy);
		GarbageEnemies.Shrink();

		JuniorEnemies.Remove(GarbageEnemy);
	}
	BuildOrbitStructure();
}
