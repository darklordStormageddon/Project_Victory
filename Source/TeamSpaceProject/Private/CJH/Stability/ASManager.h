// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASManager.generated.h"

class AASCore;
class AASBody;
class AASWing;

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
	float min_Wing = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float max_Wing = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float MinRandomFirstDist = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float MaxRandomFirstDist = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	float Spawn_Distance = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TSubclassOf<AActor> Center;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TSubclassOf<AASCore> Core;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TArray<TSubclassOf<AASBody>> Bodies;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TArray<TSubclassOf<AASWing>> Wings;

private:
	void Artifical_Satellite_Core_Spawn();

public:
	float CorrectWingNum();
	int GetBodiesNum();
	int GetWingsNum();
	
	float GetRandomFirstDist(bool IsMin);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	AASBody* Artifical_Satellite_Body_Spawn(FVector Spawn_Location, FRotator Spawn_Rotation, int Value);
	AASWing* Artifical_Satellite_Wing_Spawn(FVector Spawn_Location, FRotator Spawn_Rotation, float RestNum, int Value, bool Direction);

};
