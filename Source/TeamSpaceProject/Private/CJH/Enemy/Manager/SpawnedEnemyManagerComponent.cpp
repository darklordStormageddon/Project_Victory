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
	Super::SpawnEnemy(Enemy, _enemyInfo, SpawnLocation, SpawnRotator);

}