// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Particles/ParticleSystemComponent.h"

#include "Kismet/GameplayStatics.h"

#include "EnemyBase.generated.h"

class UEnemyManagerComponent;
class USpaceObjectComponent;
class UHealthComponent;

USTRUCT()
struct FEnemyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float MinSize;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float MaxSize;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Max_HP;
	float Current_HP;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Attack_Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Attack_Speed;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Attack_Range;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Detection_Range;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Move_Speed;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Value;
};

UCLASS()
class AEnemyBase : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "SpaceObject")
	USpaceObjectComponent* SpaceObjectComp;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* DeathParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	UParticleSystemComponent* FireComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Garbage")
	TSubclassOf<AActor> EnemyGarbage;

	AActor* _owner;
	UActorComponent* _ownerComponent;

	AActor* _spaceShip;

	bool Murdered = false;

	float delayTime;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FEnemyInfo _spawnedInfo;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	AActor* Target = nullptr;

	UEnemyManagerComponent* EnemyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;

	float Size;

public:	
	// Sets default values for this actor's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool DistanceCheck(float _condition);

	void SetInfo();

	UFUNCTION()
	void EnemyDeath();
	void SpaceObject_Remove();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	virtual void Tick(float DeltaTime) override;
	void SetTargetShip(TSubclassOf<AActor> Target);

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
