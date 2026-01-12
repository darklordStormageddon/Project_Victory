// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CJH/Enemy/EnemyBase.h"

#include "EnemyManagerComponent.generated.h"

class AJHSGameMode;
class ASpaceStation;

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
	float Attack_Speed;
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Move_Speed;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float Value;
};
//
//UENUM(BlueprintType)
//enum class EnemyClass
//{
//	Basic_Turret	UMETA(DisplayName = "Basic_Turret"),
//	Space_Drone	UMETA(DisplayName = "Drone"),
//	Enemy_C	UMETA(DisplayName = "Turret_C"),
//	Enemy_D	UMETA(DisplayName = "Turret_D"),
//};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEnemyManagerComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	UPROPERTY(EditDefaultsOnly, Category = "TargetShip")
	TSubclassOf<AActor> _spaceShip;

	// 소환 될 공간 반지름
	AJHSGameMode* _gameMode;
	ASpaceStation* _spaceStation;
	float _spaceRadius;

	// 적 소환 가능 여부
	FTimerHandle SpawnHandle;
	bool CanSpawn = true;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float minSize = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float maxSize = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float SpawnDelay = 3.0f;

	// 적 정보 맵
	UPROPERTY(EditDefaultsOnly, Category = "EnemyInfo")
	TMap<TSubclassOf<AEnemyBase>, FSpawnEnemyInfo> _enemyInfoMap;

private:
	// 적 소환 함수
	void SpawnSetting();
	void SetCanSpawnTrue();
	void SpawnEnemy(TSubclassOf<AEnemyBase> Enemy, FSpawnEnemyInfo _enemyInfo, FVector SpawnLocation, FRotator SpawnRotator);

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
