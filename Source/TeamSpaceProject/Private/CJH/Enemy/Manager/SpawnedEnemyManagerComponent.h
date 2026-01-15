// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Manager/EnemyManagerComponent.h"
#include "SpawnedEnemyManagerComponent.generated.h"

class AJHSGameMode;
class ASpaceStation;
class AEnemyBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class USpawnedEnemyManagerComponent : public UEnemyManagerComponent
{
	GENERATED_BODY()

protected:
	// Àû Á¤º¸ ¸Ê
	UPROPERTY(EditDefaultsOnly, Category = "EnemyInfo")
	TMap<TSubclassOf<AEnemyBase>, FSpawnEnemyInfo> _EnemyInfoMap;

	bool CanSpawn = true;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float SpawnDelay = 3.0f;

protected:
	void SpawnSetting();

	virtual void BeginPlay();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SpawnInMap(FVector SpawnLocation, FRotator SpawnRotation, TMap<TSubclassOf<AEnemyBase>, FSpawnEnemyInfo> _spawnInfo);
	virtual void SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator) override;

	void SetCanSpawnTrue() {
		CanSpawn = true;
	}

};
