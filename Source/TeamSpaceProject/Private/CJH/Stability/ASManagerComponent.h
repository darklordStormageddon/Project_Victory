// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ASManagerComponent.generated.h"

class ASatellite_Base;
class ASpaceStation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSatelliteSpawned, ASatellite_Base*, Satellite);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UASManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UASManagerComponent();

	UFUNCTION(BlueprintCallable)
	void StartSpawn();

	UFUNCTION(BlueprintCallable)
	void StopSpawn();

	UFUNCTION(BlueprintCallable)
	void ClearSpawnedSatellites();

	const TArray<TWeakObjectPtr<ASatellite_Base>>& GetSpawnedSatellites() const { return SpawnedSatellites; }

	UPROPERTY(BlueprintAssignable, Category = "Artifical_Satellite_Spawn")
	FOnSatelliteSpawned OnSatelliteSpawned;

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	bool bAutoStart = true;

	float Spawn_Distance = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Artifical_Satellite_Spawn")
	TArray<TSubclassOf<ASatellite_Base>> Artifical_Satellite;

	ASpaceStation* spaceStation = nullptr;

	int32 Target_Spawn_Count = 0;
	int32 Spawned_Count = 0;

	FTimerHandle Spawn_TimerHandle;

	TArray<TWeakObjectPtr<ASatellite_Base>> SpawnedSatellites;

private:
	void Artifical_Satellite_Spawn();
	void GetSetting();
	void SpawnNextArtificialSatellite();
};
