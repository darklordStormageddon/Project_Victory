// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyRoundManagerComponent.generated.h"

class ASatellite_Base;
class AEnemyBase;
class UEnemySpawnComponent;
class UGarbageEnemySpawnComponent;
class UASManagerComponent;

USTRUCT(BlueprintType)
struct FEnemySpawnGroup
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	TSubclassOf<AEnemyBase> EnemyTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 MinCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 MaxCount = 0;
};

USTRUCT(BlueprintType)
struct FRoundEnemySettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	TArray<FEnemySpawnGroup> Groups;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 MaxTotal = 5;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEnemyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyManagerComponent();

	UFUNCTION(BlueprintCallable)
	void StartRound(int32 Round);

	UFUNCTION(BlueprintCallable)
	void EndRound();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FDelegateHandle OnStartStageHandle;
	FDelegateHandle OnEndStageHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	bool bAutoStart = true;

	UPROPERTY(EditDefaultsOnly, Category = "Round", meta = (ClampMin = "1"))
	int32 AutoStartStage = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	TArray<FRoundEnemySettings> RoundSettings;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 FixedEnemyCountAfterRound5 = 5;

	int32 CurrentRound = 0;

	UASManagerComponent* SatelliteManager = nullptr;

	TArray<TWeakObjectPtr<ASatellite_Base>> SpawnedSatellites;

private:
	UFUNCTION()
	void HandleSatelliteSpawned(ASatellite_Base* Satellite);
	void SpawnEnemiesForSatellite(ASatellite_Base* Satellite);
	FRoundEnemySettings GetRoundSettings(int32 Round) const;
	void BuildSpawnList(const FRoundEnemySettings& Settings, int32 Round, TArray<TSubclassOf<AEnemyBase>>& OutSpawnList) const;
	void ClearRoundActors();
	void SpawnForExistingSatellites();
};
