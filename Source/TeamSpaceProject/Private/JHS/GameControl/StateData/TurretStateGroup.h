// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTurretStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTurretStateGroup();

private:
	const int32 HUNDRED = 100;

	TObjectPtr<AJHSGameState> _gameState = nullptr;

	TMap<int32, FTurretData> _turretDataMap;

	TMap<E_AMMO_TYPE, FAmmoData> _ammoDataMap;

	TMap<E_TURRET_POSITION, FTurretData> _equipTurretMap;

	const int32 CONSUME_AMMO = -1;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeTurretState(TObjectPtr<AJHSGameState> GameState, TArray<FAmmoData> AmmoDataArray);

	void UpdateTurretState();

	bool TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType);

	bool TryGetTurretFireInterval(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime);

	bool TryFireTurret(E_TURRET_POSITION TurretPosition);

	bool TryReloadTurret(E_TURRET_POSITION TurretPosition);

private:
	void LoadTurretDataTable();

	int32 GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	bool TryGetEquipedTurret(E_TURRET_POSITION TurretPosition, FTurretData*& OutTurretData);

	bool TryGetTurretData(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData);

	void ChangeTurretAmmo(FTurretData* TurretData, int32 ChangeValue);
};
