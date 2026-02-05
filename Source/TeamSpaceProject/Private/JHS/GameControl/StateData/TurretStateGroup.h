// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretStateGroup.generated.h"

class AJHSGameState;
class ATurretStand;
class ATurretChair;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTurretStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTurretStateGroup();

private:
	const int32 HUNDRED = 100;

	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY()
	TMap<E_TURRET_POSITION, TObjectPtr<ATurretStand>> _turretStandMap;

	UPROPERTY()
	TMap<int32, FTurretData> _turretDataMap;

	const int32 CONSUME_AMMO = -1;

	bool _isInfiniteMagMode = false;

	UPROPERTY();
	TObjectPtr<ATurretChair> _turretChair = nullptr;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeTurretState(TObjectPtr<AJHSGameState> GameState);

	void UpdateTurretState();

	void SetInfiniteMagMode(bool IsInfiniteMagMode);

	void TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType);

	UFUNCTION(Server, Reliable)
	void ServerEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, AActor* SpawnedTurret);

	bool TryGetTurretFireInterval(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime);

	bool TryFireTurret(E_TURRET_POSITION TurretPosition);

	bool TryReloadTurret(E_TURRET_POSITION TurretPosition);

private:
	void LoadTurretDataTable();

	int32 GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	bool TryGetTurretData(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData);

	bool TryGetTurretStand(E_TURRET_POSITION TurretPosition, TObjectPtr<ATurretStand>& OutTurretStand);

	void ChangeTurretAmmo(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, int32 ChangeValue);

	void ExecuteTurretEvent(E_TURRET_POSITION TurretPosition, FTurretData TurretData);
};
