// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Manager/SpawnedEnemyManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "CJH/Enemy/Base/SpawnedEnemyBase.h"

void USpawnedEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnSetting();
}

void USpawnedEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 클라이언트: 스킵
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// 스폰 대기 중일 때만 처리
	if (CanSpawn && _EnemyInfoMap.Num() > 0)
		SpawnSetting();
}

void USpawnedEnemyManagerComponent::SpawnInMap(FVector SpawnLocation, FRotator SpawnRotation, TMap<TSubclassOf<AEnemyBase>, FSpawnEnemyInfo> _spawnInfo)
{
	if (_spawnInfo.Num() == 0)
		return;

	// 랜덤 인덱스 계산
	int32 SpawnEnemyNum = FMath::RandRange(0, _spawnInfo.Num() - 1);

	int32 Index = 0;

	// 맵 반복 최적화
	for (const auto& Elem : _spawnInfo)
	{
		if (Index == SpawnEnemyNum)
		{
			if (!Elem.Key)
				return;

			SpawnEnemy(Elem.Key, Elem.Value, SpawnLocation, SpawnRotation);
			return;
		}
		Index++;
	}
}

//스폰 세팅
void USpawnedEnemyManagerComponent::SpawnSetting()
{
	CanSpawn = false;

	if (!_owner)
		return;

	FVector CenterLocation = _owner->GetActorLocation();
	FVector RandomDirection = FMath::VRand();
	FVector SpawnLocation = CenterLocation + RandomDirection * _spaceRadius;

	// 회전 최적화: 필요한 축만 랜덤화
	FRotator SpawnRotation = FRotator::ZeroRotator;

	SpawnInMap(SpawnLocation, SpawnRotation, _EnemyInfoMap);

	// 타이머 설정 (반복 실행 대신 한 번만 설정)
	GetWorld()->GetTimerManager().SetTimer(
		SpawnHandle,
		this,
		&USpawnedEnemyManagerComponent::SetCanSpawnTrue,
		SpawnDelay,
		false
	);
}

void USpawnedEnemyManagerComponent::SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator)
{
	// 서버에서만 실행
	if (!GetOwner()->HasAuthority())
		return;

	AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
		Enemy,
		SpawnLocation,
		SpawnRotator
	);

	if (!NewEnemy)
		return;

	// 배열 추가
	_spawnedEnemies.Add(NewEnemy);

	// 에너미 정보 설정
	FTargetInfo TargetInfo;
	TargetInfo.Size = FMath::RandRange(_enemyInfo.MinSize, _enemyInfo.MaxSize);
	TargetInfo.Max_HP = _enemyInfo.Max_HP;
	TargetInfo.Attack_Damage = _enemyInfo.Attack_Damage;
	TargetInfo.Speed = _enemyInfo.Move_Speed;

	FEnemyInfo EnemyInfo;
	EnemyInfo.Attack_Range = _enemyInfo.Attack_Range;
	EnemyInfo.Detection_Range = _enemyInfo.Detection_Range;
	EnemyInfo.Attack_Speed = _enemyInfo.Attack_Speed;

	// 한 번의 함수 호출로 정보 설정
	NewEnemy->SetEnemyInfo(TargetInfo, EnemyInfo);
	NewEnemy->SetTargetShip(_spaceShip);
	NewEnemy->OwnerGET(_owner);
	NewEnemy->ComponentGET(this);
}

void USpawnedEnemyManagerComponent::RemoveEnemies(AEnemyBase* _removeEnemy)
{ }