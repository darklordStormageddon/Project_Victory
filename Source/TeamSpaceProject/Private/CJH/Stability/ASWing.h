// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASWing.generated.h"

UCLASS()
class AASWing : public AActor
{
	GENERATED_BODY()
public:
	float RestWing;
	float Numbering;

private:
	void Spawn_Wing();

public:
	// Sets default values for this actor's properties
	AASWing();
	bool Direction;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
