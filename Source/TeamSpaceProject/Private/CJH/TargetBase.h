// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KSM/HealthComponent.h"

#include "TargetBase.generated.h"

USTRUCT()
struct FSpawnGarbage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Garbage")
	TSubclassOf<AActor> EnemyGarbage;

	UPROPERTY(EditAnywhere, Category = "Garbage")
	int Min_SpawnNum;

	UPROPERTY(EditAnywhere, Category = "Garbage")
	int Max_SpawnNum;

};

USTRUCT()
struct FGarbageInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Garbage")
	TArray<FSpawnGarbage> GarbageList;

	UPROPERTY(EditAnywhere, Category = "Garbage")
	int Total_Number;
};


USTRUCT()
struct FTargetInfo
{
	GENERATED_BODY()

	float Size;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Max_HP;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Attack_Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Speed;
};

UCLASS()
class ATargetBase : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	FTargetInfo _targetInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool DebugRevive;
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	float RiviveTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Garbage")
	UDataTable* GarbageTable;

	UPROPERTY(EditDefaultsOnly, Category = "Garbage")
	FName GarbageRowName;

public:	
	// Sets default values for this actor's properties
	ATargetBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SpawnGarbageSetting();
	void Spawn(TSubclassOf<AActor> EnemyGarbage);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
