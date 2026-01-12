// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASCore.generated.h"

class ASpaceStation;

UCLASS()
class AASCore : public AActor
{
	GENERATED_BODY()
private:
	struct FInfo
	{
		float Speed;
		FVector Direction;
	};

private:
	bool bChangeDirection = false;

	UPROPERTY(EditDefaultsOnly, Category = "Delay")
	float ResetChangeDelay = 3.f;

public:
	FInfo Info;

private:
	void Move(float DeltaTime);
	void DistanceCheck();
	void ReSetVector(ASpaceStation* SpaceStation, float Radius);
	void ResetChangeDirection();

public:
	AASCore();
	virtual void Tick(float DeltaTime) override;
};
