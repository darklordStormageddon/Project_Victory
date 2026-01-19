// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"


#include "Particles/ParticleSystemComponent.h"

#include "Kismet/GameplayStatics.h"

#include "EnemyBase.generated.h"

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
private:
	float delayTime;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	UParticleSystem* FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	UParticleSystemComponent* FireComponent;

	AActor* _owner;
	UActorComponent* _ownerComponent;

	AActor* _spaceShip;



public:
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FEnemyInfo _spawnedInfo;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	AActor* Target = nullptr;

	float Size;

private:
	void SetInfo();

public:	
	// Sets default values for this actor's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool DistanceCheck(float _condition);

public:
	virtual void Tick(float DeltaTime) override;
	void SetTargetShip(TSubclassOf<AActor> Target);

public:
	void OwnerGET(AActor* _getOwner) { _owner = _getOwner; }
	void ComponentGET(UActorComponent* _getOwner) { _ownerComponent = _getOwner; }
};
