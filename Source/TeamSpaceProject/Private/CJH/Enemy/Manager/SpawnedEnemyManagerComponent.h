// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Manager/EnemyManagerComponent.h"
#include "SpawnedEnemyManagerComponent.generated.h"

class AJHSGameMode;
class ASpaceStation;
class AEnemyBase;

USTRUCT(BlueprintType)

struct FSpawnEnemyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Max_HP = 100.f;

	float Current_HP = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Damage = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Range = 5000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Detection_Range = 10000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Speed = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Move_Speed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Value = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float MinSize = 0.1f;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float MaxSize = 1.f;
};

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
	void SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator);

	void SetCanSpawnTrue() {
		CanSpawn = true;
	}

public:
	virtual void RemoveEnemies(AEnemyBase* _removeEnemy) override;

};
