// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASManager.generated.h"

class ASatellite_Base;
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

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float Spawn_Interval = 0.5f;

	float Spawn_Distance;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TArray<TSubclassOf<ASatellite_Base>> Artifical_Satellite;

	ASpaceStation* spaceStation = nullptr;

private:
	void Artifical_Satellite_Spawn();
	void GetSetting();
	void SpawnNextArtificialSatellite();

	int Target_Spawn_Count = 0;
	int Spawned_Count = 0;

	FTimerHandle Spawn_TimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
