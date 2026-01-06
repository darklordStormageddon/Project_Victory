// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASCore.generated.h"

class AASManager;

UCLASS()
class AASCore : public AActor
{
	GENERATED_BODY()

private:
	void SpawnBody();

	UPROPERTY()
	AASManager* Manager;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
