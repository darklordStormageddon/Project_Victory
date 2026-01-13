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

	TMap<int32, FPlayerStateData> _playerStateMap;
	
protected:
	// SpaceShip
	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	FSpaceShipState _spaceShipState;

	// Player Radiation
	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	float _maxPlayerRadiation = 100.0f;

	// Turret
	const int32 _consumeAmmo = -1;

	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	FTurretData _turretData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test")
	int32 _testPlayerCount = 4;

public:
	int32 GetPlayerCount() { return _playerStateMap.Num(); }

protected:
	virtual void BeginPlay() override;

public:
	void InitializeGameState(TArray<FPlayerStateData> PlayerStateArray);

	void SendCurrentDataEvent();

#pragma region SpaceShip
public:
	void RepairSpaceShip();

	void DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue);

	void RepairShield(float RepairShieldValue);

private:
	void ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue);

	void ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue, float MaxValue);
#pragma endregion SpaceShip

#pragma region Player State
public:
	void IncreasePlayerRadiation(int32 PlayerIdx, float IncreaseValue);
#pragma endregion Player State

#pragma region Turret
public:
	float GetTurretFireCoolTime() { return _turretData.FireCoolTime; }

	bool TryFireTurret();

	void ReloadTurret();

private:
	void ChangeTurretAmmo(int32 ChangeValue);
#pragma endregion Turret

private:
	TObjectPtr<UEventManager> GetEventManager();

	
};
