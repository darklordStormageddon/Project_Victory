// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "JHSGameState.generated.h"

class UEventManager;
class USpaceShipStateGroup;
class UPlayerStateGroup;
class UCollectStateGroup;
class UContainerStateGroup;
class UTurretStateGroup;

UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()

public:
	AJHSGameState();

private:
	UPROPERTY()
	TObjectPtr<UEventManager> _cachedEventManager = nullptr;

private:
	UPROPERTY(EditAnywhere, Category = "GameState|Goal")
	int32 _goalDollar = 1000;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

#pragma region State Group
protected:
	// SpaceShip
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|SpaceShipStateGroup")
	TObjectPtr<USpaceShipStateGroup> _spaceShipStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|SpaceShipStateGroup")
	FSpaceShipState _initSpaceShipState;

	// Player
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|PlayerStateGroup")
	TObjectPtr<UPlayerStateGroup> _playerStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|PlayerStateGroup")
	float _maxPlayerRadiation = 100.0f;

	// Collect
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|CollectStateGroup")
	TObjectPtr<UCollectStateGroup> _collectStateGroup = nullptr;

	// Container
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|ContainerStateGroup")
	TObjectPtr<UContainerStateGroup> _containerStateGroup = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameState|ContainerStateGroup")
	FContainerState _initContainerState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameState|ContainerStateGroup")
	TArray<FAmmoData> _initAmmoDataArray;

	// Turret
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|TurretStateGroup")
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;
#pragma endregion State Group

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test")
	int32 _testPlayerCount = 4;

public:
	int32 GetGoalDollar() { return _goalDollar; }

public:
	TObjectPtr<USpaceShipStateGroup> GetSpaceShipStateGroup() { return _spaceShipStateGroup; }

	TObjectPtr<UPlayerStateGroup> GetPlayerStateGroup() { return _playerStateGroup; }

	TObjectPtr<UCollectStateGroup> GetCollectStateGroup() { return _collectStateGroup; }

	TObjectPtr<UContainerStateGroup> GetContainerStateGroup() { return _containerStateGroup; }

	TObjectPtr<UTurretStateGroup> GetTurretStateGroup() { return _turretStateGroup; }

protected:
	virtual void BeginPlay() override;

public:
	void InitializeGameState(TArray<FPlayerStateData> PlayerStateArray);

	void SendCurrentDataEvent();

public:
	TObjectPtr<UEventManager> GetEventManager();
};
