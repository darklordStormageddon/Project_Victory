// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JHS/GameControl/DataStruct.h"

#include "JHSGameState.generated.h"

class UEventManager;

UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()

public:
	AJHSGameState();

private:
	TObjectPtr<UEventManager> _cachedEventManager = nullptr;
	
protected:
	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	FSpaceShipData _spaceShipData;

private:
	TObjectPtr<UEventManager> GetEventManager();

	void ChangeSpaceShipData(FMaxCurrentData* OriginalData, float CurrentValue);

	void ChangeSpaceShipData(FMaxCurrentData* OriginalData, float CurrentValue, float MaxValue);

public:
	void SendCurrentDataEvent();

	void RepairSpaceShip();

	void DecreaseSpaceShipData(E_DATA_TYPE DataType, float DecreaseValue);

	void RepairShield(float RepairShieldValue);
};
