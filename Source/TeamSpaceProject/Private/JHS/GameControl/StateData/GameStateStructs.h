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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSpaceShipData Shield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSpaceShipData HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FAmmoData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_AMMO_TYPE AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ReloadCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AmmoDamage;

	UPROPERTY()
	int32 AmmoStockpile;

	UPROPERTY()
	UTexture2D* AmmoImage = nullptr;
};

UENUM(BlueprintType)
enum class E_TURRET_POSITION : uint8
{
	Main = 0 UMETA(DisplayName = "Main"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),

	END UMETA(DisplayName = "END"),
};

USTRUCT(BlueprintType)
struct FTurretData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString TurretBPName;

	UPROPERTY()
	E_AMMO_TYPE AmmoType;

	UPROPERTY()
	FMaxCurrentData Mag;

	UPROPERTY()
	float FireInterval = 1.0f;
};
#pragma endregion Turret

#pragma region Container
UENUM(BlueprintType)
enum class E_ELEMENT_TYPE : uint8
{
	// 알루미늄, 철, 티타늄, 니켈, 구리, 실리콘, 금, 리튬, 탄소섬유
	Aluminum = 0 UMETA(DisplayName = "Aluminum"),
	Iron UMETA(DisplayName = "Iron"),
	Titanium UMETA(DisplayName = "Titanium"),
	Nickel UMETA(DisplayName = "Nickel"),
	Copper UMETA(DisplayName = "Copper"),
	Silicon UMETA(DisplayName = "Silicon"),
	Gold UMETA(DisplayName = "Gold"),
	Lithium UMETA(DisplayName = "Lithium"),
	CarbonFiber UMETA(DisplayName = "CarbonFiber"),

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FElementData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_ELEMENT_TYPE ElementType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ValueOfElement;

	UPROPERTY()
	int32 Amount;

	UPROPERTY()
	UTexture2D* ElementImage = nullptr;
};

USTRUCT(BlueprintType)
struct FContainerState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Money;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxElementStockpile;

	UPROPERTY()
	TMap<E_ELEMENT_TYPE, FElementData> ElementDataMap;

	UPROPERTY()
	TMap<E_AMMO_TYPE, FAmmoData> AmmoDataMap;
};
#pragma endregion Container
