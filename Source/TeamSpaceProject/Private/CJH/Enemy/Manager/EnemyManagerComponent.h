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

USTRUCT(BlueprintType)

struct FSpawnEnemyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Max_HP;

	float Current_HP;
	
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Damage;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Range;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Detection_Range;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Attack_Speed;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Move_Speed;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Value;
};

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

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float minSize = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float maxSize = 1.5f;

	//Garbage
	float SpawnedValue;

	AEnemyBase* SpawnedEnemy;
		
protected:
	// 적 소환 함수
	void SpawnSetting();
	void GarbageSpawnSetting();

	virtual void SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator);

public:	
	// Sets default values for this component's properties
	UEnemyManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
