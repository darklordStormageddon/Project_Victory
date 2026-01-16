// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Manager/EnemyManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "CJH/Enemy/Base/SpawnedEnemyBase.h"

UEnemyManagerComponent::UEnemyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = GetOwner();

	_gameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());
	_spaceRadius = _gameMode->GetSpaceRadius();
	_spaceStation = _gameMode->GetSpaceStation();	
}

void UEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UEnemyManagerComponent::SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator)
{
	SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Enemy, SpawnLocation, SpawnRotator);

	_spawnedEnemies.Add(SpawnedEnemy);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->_spawnedInfo.Size = FMath::RandRange(_enemyInfo.MinSize, _enemyInfo.MaxSize);

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
