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

	if (CanSpawn && _EnemyInfoMap.Num() > 0)
		SpawnSetting();
}

void USpawnedEnemyManagerComponent::SpawnInMap(FVector SpawnLocation, FRotator SpawnRotation, TMap<TSubclassOf<AEnemyBase>, FSpawnEnemyInfo> _spawnInfo)
{
	float SpawnEnemyNum = FMath::RandRange(0, _spawnInfo.Num() - 1);

	int Index = 0;

	for (auto& Elem : _spawnInfo)
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

	FVector CenterLocation = _owner->GetActorLocation();

	FVector RandomDirection = FMath::VRand();

	FVector SpawnLocation = CenterLocation + RandomDirection * _spaceRadius;

	FRotator SpawnRotation = FRotator(
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360)
	);

	SpawnInMap(SpawnLocation, SpawnRotation, _EnemyInfoMap);

	GetWorld()->GetTimerManager().SetTimer(SpawnHandle, this, &USpawnedEnemyManagerComponent::SetCanSpawnTrue, SpawnDelay, false);
}

void USpawnedEnemyManagerComponent::SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator)
{
	SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Enemy, SpawnLocation, SpawnRotator);

	_spawnedEnemies.Add(SpawnedEnemy);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->Size = FMath::RandRange(_enemyInfo.MinSize, _enemyInfo.MaxSize);

		SpawnedEnemy->_spawnedInfo.Max_HP = _enemyInfo.Max_HP;
		SpawnedEnemy->_spawnedInfo.Current_HP = _enemyInfo.Current_HP;
		SpawnedEnemy->_spawnedInfo.Attack_Damage = _enemyInfo.Attack_Damage;
		SpawnedEnemy->_spawnedInfo.Attack_Speed = _enemyInfo.Attack_Speed;
		SpawnedEnemy->_spawnedInfo.Attack_Range = _enemyInfo.Attack_Range;
		SpawnedEnemy->_spawnedInfo.Detection_Range = _enemyInfo.Detection_Range;
		SpawnedEnemy->_spawnedInfo.Move_Speed = _enemyInfo.Move_Speed;

		SpawnedEnemy->_spawnedInfo.Value = _enemyInfo.Value;

		SpawnedEnemy->SetTargetShip(_spaceShip);
		SpawnedEnemy->OwnerGET(_owner);
		SpawnedEnemy->ComponentGET(this);
	}
}
