// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "JHSGameState.generated.h"

class UEventManager;
class USpaceShipStateGroup;
class UPlayerStateGroup;
class UTurretStateGroup;

UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()

public:
	AJHSGameState();

private:
	TObjectPtr<UEventManager> _cachedEventManager = nullptr;
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|SpaceShipStateGroup")
	TObjectPtr<USpaceShipStateGroup> _spaceShipStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|SpaceShipStateGroup")
	FSpaceShipState _initSpaceShipState;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|PlayerStateGroup")
	TObjectPtr<UPlayerStateGroup> _playerStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|PlayerStateGroup")
	float _maxPlayerRadiation = 100.0f;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|TurretStateGroup")
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|TurretStateGroup")
	FTurretData _initTurretData;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test")
	int32 _testPlayerCount = 4;

public:
	TObjectPtr<USpaceShipStateGroup> GetSpaceShipStateGroup() { return _spaceShipStateGroup; }

	TObjectPtr<UPlayerStateGroup> GetPlayerStateGroup() { return _playerStateGroup; }

	TObjectPtr<UTurretStateGroup> GetTurretStateGroup() { return _turretStateGroup; }

protected:
	virtual void BeginPlay() override;

public:
	void InitializeGameState(TArray<FPlayerStateData> PlayerStateArray);

	void SendCurrentDataEvent();

public:
	TObjectPtr<UEventManager> GetEventManager();
};
