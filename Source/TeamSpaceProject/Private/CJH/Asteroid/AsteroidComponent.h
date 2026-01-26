// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CJH/Asteroid/Asteroid.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AsteroidComponent.generated.h"

USTRUCT()
struct FSpawnAsteroidInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Asteroid Spawn")
	bool debugDraw = false;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Max_HP;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MinSpawnDelay = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MaxSpawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MinSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MaxSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MinSize;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MaxSize;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	TArray<TSubclassOf<AAsteroid>> AsteroidClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	TSubclassOf<AActor> TargetShip;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class UAsteroidComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	FTimerHandle SpawnTimerHandle;

	AActor* _ownerActor = nullptr;

	bool bIsSpawning = false;

	FVector ShipSpeed;

	UPROPERTY()
	TArray<AAsteroid*> Asteroids;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	FSpawnAsteroidInfo _asteroidInfo;

private:
	void CanSpawn();
	void SpawnAsteroid();

	float SetDamage(float Speed, float Size);

public:
	// Sets default values for this component's properties
	UAsteroidComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RemoveAsteroid(AAsteroid* _removeTarget);

};
 