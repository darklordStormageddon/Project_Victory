// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

class UAsteroidComponent;

UCLASS()

class AAsteroid : public AActor
{
	GENERATED_BODY()
private:
	float MoveDistance;
	
public:
	typedef struct FAsteroidInfo
	{
		float Speed;
		float Size;
		float Health;
		float Damage;
	} FAsteroidInfo;

	UAsteroidComponent* AsteroidComponent;
private:
	FAsteroidInfo AsteroidInfo;

	FVector Direction;

	FRotator ConstRotaion;
	float RotateSpeed;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float MinRotateSpeed = 5.0f; 
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float MaxRotateSpeed = 20.0f;

private:
	void MoveAsteroid(float DeltaTime);
	void SetAsteroidRot();

	UFUNCTION()
	void DestroyAsteroid();
public:
	// Sets default values for this actor's properties
	AAsteroid();

	float DestroyDistance;
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
