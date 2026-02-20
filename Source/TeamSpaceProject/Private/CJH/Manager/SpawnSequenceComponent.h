// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnSequenceComponent.generated.h"

class UStaticAsternoidManagerComponent;
class UASManagerComponent;
class UEnemyManagerComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class USpawnSequenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpawnSequenceComponent();

	UFUNCTION(BlueprintCallable)
	void StartSequence(int32 Round);

	UFUNCTION(BlueprintCallable)
	void EndSequence();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sequence")
	bool bAutoStart = false;

	UPROPERTY(EditDefaultsOnly, Category = "Sequence", meta = (ClampMin = "1"))
	int32 AutoStartStage = 1;

	int32 CurrentRound = 0;

	UPROPERTY()
	UStaticAsternoidManagerComponent* StaticAsteroidManager = nullptr;

	UPROPERTY()
	UASManagerComponent* SatelliteManager = nullptr;

	UPROPERTY()
	UEnemyManagerComponent* EnemyManager = nullptr;

	FDelegateHandle OnStartStageHandle;
	FDelegateHandle OnEndStageHandle;

	UFUNCTION()
	void OnStaticAsteroidComplete();

	UFUNCTION()
	void OnAllSatellitesComplete();
};
