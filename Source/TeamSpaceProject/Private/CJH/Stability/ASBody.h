// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CJH/Stability/ASWing.h"

#include "ASBody.generated.h"

class AASManager;

UCLASS()
class AASBody : public AActor
{
	GENERATED_BODY()
	
private:
	enum Direction {
		RightWing = 1,
		LeftWing = -1
	};

	void SpawnFirstWing();
	void WingArrow(Direction wArrow);
	float AttachDist(float BodyRadius);

	int WingsNum;
	int RestWing;

	AASManager* Manager;

	AASWing* WingActor = nullptr;

private:
	void Call_WingSpawn();

public:
	float WingDist();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
