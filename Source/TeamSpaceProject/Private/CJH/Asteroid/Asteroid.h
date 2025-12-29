// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

UCLASS()

class AAsteroid : public AActor
{
	GENERATED_BODY()

public:
	typedef struct FAsteroidInfo
	{
		float Speed;
		float Size;
		float Health;
		float Damage;
	} FAsteroidInfo;

private:
	FAsteroidInfo AsteroidInfo;

	FVector Direction;
public:
	// Sets default values for this actor's properties
	AAsteroid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetAsteroidInfo(
		const FAsteroidInfo& InAsteroidInfo,
		FVector VSpaceShip,
		FVector Velocity);
};
