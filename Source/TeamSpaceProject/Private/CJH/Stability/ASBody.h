// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CJH/Stability/ASWing.h"

#include "ASBody.generated.h"

UCLASS()
class AASBody : public AActor
{
	GENERATED_BODY()
	
private:
	enum Direction {
		RightWing = 1,
		LeftWing = -1
	};

	UPROPERTY(EditDefaultsOnly, Category = "WingSpawn")
	float MinRandomDist = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "WingSpawn")
	float MaxRandomDist = 300.f;

	void SpawnFirstWing();
	void WingArrow(Direction wArrow);
	float AttachDist(float BodyRadius);

	int WingsNum;
	int RestWing;

	AActor* ActorManager;
	AASWing* WingActor = nullptr;
public:
	// Sets default values for this actor's properties
	AASBody();
	float WingDist();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
