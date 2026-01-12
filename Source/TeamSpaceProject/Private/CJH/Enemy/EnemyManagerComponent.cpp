// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/EnemyManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

// Sets default values for this component's properties
UEnemyManagerComponent::UEnemyManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	_gameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());
	_spaceRadius = _gameMode->GetSpaceRadius();
	_spaceStation = _gameMode->GetSpaceStation();

	SpawnSetting();
}


// Called every frame
void UEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CanSpawn && _enemyInfoMap.Num() > 0)
		SpawnSetting();
}

void UEnemyManagerComponent::SpawnSetting()
{
	CanSpawn = false;

	FVector CenterLocation = _spaceStation->GetActorLocation();

	FVector RandomDirection = FMath::VRand();

	FVector SpawnLocation = CenterLocation + RandomDirection * _spaceRadius;
	
	FRotator SpawnRotation = FRotator(
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360)
	);
	
	float SpawnEnemyNum = FMath::RandRange(0, _enemyInfoMap.Num() - 1);

	//TMap에서 랜덤으로 적 클래스 선택 후 해당 적한테 Info 할당
	TSubclassOf<AEnemyBase> EnemyClassToSpawn;
	int Index = 0;

	for (auto& Elem : _enemyInfoMap)
	{
		if (Index == SpawnEnemyNum)
		{
			if (!Elem.Key)
				break;

			SpawnEnemy(Elem.Key, Elem.Value, SpawnLocation, SpawnRotation);
			break;
		}
		Index++;
	}

	GetWorld()->GetTimerManager().SetTimer(SpawnHandle, this, &UEnemyManagerComponent::SetCanSpawnTrue, SpawnDelay, false);
}

void UEnemyManagerComponent::SetCanSpawnTrue()
{
	CanSpawn = true;
}

void UEnemyManagerComponent::SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator)
{
	AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Enemy, SpawnLocation, SpawnRotator);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->_spawnedInfo.Size = FMath::RandRange(minSize, maxSize);

		SpawnedEnemy->_spawnedInfo.Max_HP = _enemyInfo.Max_HP;
		SpawnedEnemy->_spawnedInfo.Current_HP = _enemyInfo.Current_HP;
		SpawnedEnemy->_spawnedInfo.Attack_Damage = _enemyInfo.Attack_Damage;
		SpawnedEnemy->_spawnedInfo.Attack_Speed = _enemyInfo.Attack_Speed;
		SpawnedEnemy->_spawnedInfo.Attack_Range = _enemyInfo.Attack_Range;
		SpawnedEnemy->_spawnedInfo.Move_Speed = _enemyInfo.Move_Speed;

		SpawnedEnemy->_spawnedInfo.Value = _enemyInfo.Value;

		SpawnedEnemy->SetTargetShip(_spaceShip);
	}
}
//현재 한 일
//스폰 기능 구현
//적 정보 맵을 통해 다양한 Info 할당 가능

//해야 할 일
//적 동작 구현