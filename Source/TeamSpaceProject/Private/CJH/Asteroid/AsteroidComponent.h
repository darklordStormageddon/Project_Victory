// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CJH/Asteroid/Asteroid.h"

#include "AsteroidComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class UAsteroidComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	FTimerHandle SpawnTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	float SpawnDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	float SpawnDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	float MinSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	float MaxSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Meteor Spawn")
	TArray<TSubclassOf<AAsteroid>> MeteorClasses;

	bool bIsSpawning = false;

private:
	void SpawnMeteor();
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


};
 