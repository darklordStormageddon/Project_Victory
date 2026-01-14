// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/GameControl/StateData/GameStateData.h"

#include "GameStateStructs.generated.h"

// Forward declaration
class AJHSGameState;

#pragma region SpaceShip
UENUM(BlueprintType)
enum class E_SPACE_SHIP_DATA_TYPE : uint8
{
	Shield = 0 UMETA(DisplayName = "Shield"),
	HP UMETA(DisplayName = "HP"),
	Fuel UMETA(DisplayName = "Fuel"),
};

USTRUCT(BlueprintType)
struct FSpaceShipData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_SPACE_SHIP_DATA_TYPE DataType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMaxCurrentData Values;
};

USTRUCT(BlueprintType)
struct FSpaceShipState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FSpaceShipData Shield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FSpaceShipData HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FSpaceShipData Fuel;
};
#pragma endregion SpaceShip

#pragma region Player
USTRUCT(BlueprintType)
struct FPlayerStateData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	int32 PlayerUID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	int32 PlayerIdx;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	FMaxCurrentData Radiation;
};
#pragma endregion Player

#pragma region Turret
UENUM(BlueprintType)
enum class E_AMMO_TYPE : uint8
{
	Bullet = 0 UMETA(DisplayName = "Bullet"),
	Cannon UMETA(DisplayName = "Cannon"),
	Missile UMETA(DisplayName = "Missile"),
};

USTRUCT(BlueprintType)
struct FAmmoData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoData")
	E_AMMO_TYPE AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoData")
	int32 ReloadMount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoData")
	float _ammoDamage;
};

UENUM(BlueprintType)
enum class E_TURRET_POSITION : uint8
{
	Main = 0 UMETA(DisplayName = "Main"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
};

USTRUCT(BlueprintType)
struct FTurretData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool IsMainTurret;

	UPROPERTY()
	E_TURRET_POSITION TurretPosition;

	UPROPERTY()
	E_AMMO_TYPE AmmoType;

	UPROPERTY()
	FMaxCurrentData Ammo;

	UPROPERTY()
	float FireCoolTime = 1.0f;
};
#pragma endregion Turret
