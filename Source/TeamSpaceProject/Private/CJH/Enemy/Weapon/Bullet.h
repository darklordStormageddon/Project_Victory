// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class ABullet : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	float Speed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	float BulletLifeTime = 5.0f;

	FVector Direction;

private:
	void MoveToTarget(float DeltaTime);

public:	
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetTarget(FVector _TargetLocation);
};
