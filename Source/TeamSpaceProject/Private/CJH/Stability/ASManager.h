// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASManager.generated.h"

class AASCore;
class ASpaceStation;

UCLASS()

class AASManager : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	int min_Spawn = 30;
	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	int max_Spawn = 60;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float min_Speed = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float max_Speed = 60.f;

	float Spawn_Distance;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TArray<TSubclassOf<AASCore>> Artifical_Satellite;

	ASpaceStation* spaceStation = nullptr;

private:
	void Artifical_Satellite_Spawn();
	void GetSetting();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
