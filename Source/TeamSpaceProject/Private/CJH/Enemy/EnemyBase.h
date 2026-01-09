// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Kismet/GameplayStatics.h"

#include "EnemyBase.generated.h"

USTRUCT()
struct FEnemyInfo
{
	GENERATED_BODY()

	float Size;

	float Max_HP;
	float Current_HP;
	float Attack_Damage;
	float Attack_Speed;
	float Attack_Range;
	float Move_Speed;

	float Value;
};

UCLASS()
class AEnemyBase : public APawn
{
	GENERATED_BODY()
private:
	float delayTime;

protected:
	AActor* _spaceShip;

public:
	FEnemyInfo _spawnedInfo;

private:
	void SetInfo();

public:	
	// Sets default values for this actor's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool DistanceCheck();

public:
	virtual void Tick(float DeltaTime) override;
	void SetTargetShip(TSubclassOf<AActor> Target);
};
