// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyManagerComponent.generated.h"

class AJHSGameMode;
class ASpaceStation;
class AEnemyBase;
class AGarbageEnemyBase;
class ASpawnedEnemyBase;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEnemyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	AActor* _owner = nullptr;

	TArray<AEnemyBase*> _spawnedEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "TargetShip")
	TSubclassOf<AActor> _spaceShip;

	// 소환 될 공간 반지름
	AJHSGameMode* _gameMode;
	ASpaceStation* _spaceStation;

	float _spaceRadius;

	// 적 소환 가능 여부
	FTimerHandle SpawnHandle;

	//Garbage
	float SpawnedValue;

	AEnemyBase* SpawnedEnemy;
	

protected:
	// 적 소환 함수
	void SpawnSetting();
	void GarbageSpawnSetting();

public:	
	// Sets default values for this component's properties
	UEnemyManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void RemoveEnemies(AEnemyBase* _removeEnemy) {};

	virtual void DeleteAllEnemy();
};
