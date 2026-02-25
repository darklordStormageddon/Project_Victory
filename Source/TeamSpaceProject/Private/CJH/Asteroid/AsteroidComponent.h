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
	float Max_HP = 50.f;

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
	float MinSize = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	float MaxSize = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	TArray<TSubclassOf<AAsteroid>> AsteroidClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroid Spawn")
	TSubclassOf<AActor> TargetShip = nullptr;
};

class UEventManager;
class UEventOnStartStage;
class UEventOnEndStage;
class AJHSGameMode;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class UAsteroidComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	FTimerHandle SpawnTimerHandle;

	FDelegateHandle OnStartStageHandle;
	FDelegateHandle OnEndStageHandle;

	AActor* _ownerActor = nullptr;

	AJHSGameMode* InGameMode = nullptr;

	UEventManager* EventManager = nullptr;

	bool bIsSpawning = false;
	bool bSpawningEnabled = false;

	FVector ShipSpeed;

	UPROPERTY()
	TArray<AAsteroid*> Asteroids;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	int MaxSpawn = 100;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	bool bAutoStart = false;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 AutoStartStage = 1;

	AActor* TargetShip;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float DamageDamping = 2.f;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	FSpawnAsteroidInfo _asteroidInfo;

private:
	void CanSpawn();
	void SpawnAsteroid();

	float SetDamage(float Speed, float Size);

	void HandleStartStage(UEventOnStartStage* Event);
	void HandleEndStage(UEventOnEndStage* Event);

public:
	// Sets default values for this component's properties
	UAsteroidComponent();

	UFUNCTION(BlueprintCallable)
	void StartSpawning();

	UFUNCTION(BlueprintCallable)
	void StopSpawning();

	UFUNCTION(BlueprintCallable)
	void ClearAsteroids();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RemoveAsteroid(AAsteroid* _removeTarget);

};