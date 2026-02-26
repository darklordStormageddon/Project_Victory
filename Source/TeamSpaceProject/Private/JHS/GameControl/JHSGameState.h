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
class UShopManager;

UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()

public:
	AJHSGameState();

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|Manager")
	TObjectPtr<UEventManager> _eventManager = nullptr;

private:
	UPROPERTY(EditAnywhere, Category = "GameState|Goal")
	int32 _goalDollar = 1000;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameState|Components")
	TObjectPtr<USceneComponent> _rootComponent = nullptr;

#pragma region State Group
private:
	// SpaceShip
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|SpaceShipStateGroup")
	TObjectPtr<USpaceShipStateGroup> _spaceShipStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|SpaceShipStateGroup")
	float _repairDelay = 5.0f;

	UPROPERTY(EditAnywhere, Category = "GameState|SpaceShipStateGroup")
	float _repairShieldValue = 1.0f;

	// Player
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|PlayerStateGroup")
	TObjectPtr<UPlayerStateGroup> _playerStateGroup = nullptr;

	// Collect
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|CollectStateGroup")
	TObjectPtr<UCollectStateGroup> _collectStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|CollectStateGroup")
	float _useToolradiationValue = 1.0f;

	// Container
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|ContainerStateGroup")
	TObjectPtr<UContainerStateGroup> _containerStateGroup = nullptr;

	UPROPERTY(EditAnywhere, Category = "GameState|ContainerStateGroup")
	FContainerState _initContainerState;

	// Turret
	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|TurretStateGroup")
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;
#pragma endregion State Group

	UPROPERTY(VisibleDefaultsOnly, Category = "GameState|ShopManager")
	TObjectPtr<UShopManager> _shopManager = nullptr;

public:
	int32 GetGoalDollar() { return _goalDollar; }

public:
	TObjectPtr<USpaceShipStateGroup> GetSpaceShipStateGroup() { return _spaceShipStateGroup; }

	TObjectPtr<UPlayerStateGroup> GetPlayerStateGroup() { return _playerStateGroup; }

	TObjectPtr<UCollectStateGroup> GetCollectStateGroup() { return _collectStateGroup; }

	TObjectPtr<UContainerStateGroup> GetContainerStateGroup() { return _containerStateGroup; }

	TObjectPtr<UTurretStateGroup> GetTurretStateGroup() { return _turretStateGroup; }

	TObjectPtr<UShopManager> GetShopManager() { return _shopManager; }

protected:
	virtual void BeginPlay() override;

public:
	void InitializeGameState();

	void SendCurrentDataEvent();

public:
	TObjectPtr<UEventManager> GetEventManager();

	static bool TryGetTextureFromPath(FString FolderPath, FString FileName, TObjectPtr<UTexture2D>& OutTexture);

	static FPurchaseData ParseFromDataRow(UTexture2D* Image, FPurchaseDataFormat PurchaseDataFormat);
};
