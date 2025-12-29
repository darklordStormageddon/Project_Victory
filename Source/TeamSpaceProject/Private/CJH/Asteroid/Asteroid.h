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
	typedef struct FMeteorInfo
	{
		float Speed;
		float Size;
		float Health;
		float Damage;
	} FMeteorInfo;

private:
	FMeteorInfo MeteorInfo;

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

	void SetMeteorInfo(
		const FMeteorInfo& InMeteorInfo,
		FVector VSpaceShip,
		FVector Velocity);
};
