// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemySpawnComponent.generated.h"

class ASpaceStation;
class AEnemyBase;
class ASatellite_Base;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEnemySpawnComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	AActor* _owner = nullptr;

	UPROPERTY()
	TArray<AEnemyBase*> _spawnedEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "TargetShip")
	TSubclassOf<AActor> _spaceShip;

	ASpaceStation* _spaceStation = nullptr;

	float _spaceRadius = 0.f;

	FTimerHandle SpawnHandle;

	float SpawnedValue = 0.f;

	AEnemyBase* SpawnedEnemy = nullptr;

protected:
	void SpawnSetting();
	void GarbageSpawnSetting();

	virtual void OnEnemySpawned(AEnemyBase* NewEnemy);

public:
	UEnemySpawnComponent();

	UFUNCTION(BlueprintCallable)
	virtual void SpawnEnemies(const TArray<TSubclassOf<AEnemyBase>>& EnemiesToSpawn);

	UFUNCTION(BlueprintCallable)
	virtual void ClearSpawnedEnemies();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void RemoveEnemies(AEnemyBase* _removeEnemy) {};

	virtual void DeleteAllEnemy();
};
