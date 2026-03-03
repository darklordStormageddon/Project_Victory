// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/TargetBase.h"

#include "Particles/ParticleSystemComponent.h"

#include "Kismet/GameplayStatics.h"

#include "EnemyBase.generated.h"

class UEnemySpawnComponent;
class USpaceObjectComponent;

USTRUCT()
struct FEnemyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float MinSize = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float MaxSize = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Attack_Speed = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Attack_Range = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Detection_Range = 10000.f;
};

UCLASS()
class AEnemyBase : public ATargetBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowStatsDebug = false;

	FVector NewScale;

	UPROPERTY(EditDefaultsOnly, Category = "SpaceObject")
	USpaceObjectComponent* SpaceObjectComp;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* DeathParticle;

	AActor* _owner;
	UActorComponent* _ownerComponent;

	UPROPERTY()
	AActor* _spaceShip;

	bool Murdered = false;

	float delayTime = 1.f;

	FTimerHandle CanDistanceHandle;

	bool CanCheck = true;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FEnemyInfo _spawnedInfo;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	AActor* Target = nullptr;

	UPROPERTY()
	UEnemySpawnComponent* EnemyComponent;

private:
	void DebugShowStat();

public:	
	// Sets default values for this actor's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetInfo();

	bool DistanceCheck(float _condition);

	UFUNCTION()
	void EnemyDeath();
	void SpaceObject_Remove();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool TargetHPCheck();
	void CanCheckDistance() { CanCheck = true; };

public:
	virtual void Tick(float DeltaTime) override;
	void SetTargetShip(TSubclassOf<AActor> Target);
	void SetEnemyInfo(
		const FTargetInfo& InEnemyInfo,
		const FEnemyInfo& InSpawnedInfo);

public:
	void OwnerGET(AActor* _getOwner) { _owner = _getOwner; }
	void ComponentGET(UActorComponent* _getOwner) { _ownerComponent = _getOwner; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool MinusDebug = false;

	bool DelayBool = true;
	float DelayTime;

	void MinusHp();
};
