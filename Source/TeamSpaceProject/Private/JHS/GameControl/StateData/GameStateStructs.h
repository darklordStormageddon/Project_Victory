// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/GameControl/StateData/GameStateData.h"

#include "GameStateStructs.generated.h"

// Forward declaration
class AJHSGameState;

// FSpaceShipState 구조체 정의 (SpaceShipStateGroup.h에서 가져옴)
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

// FPlayerStateData 구조체 정의 (PlayerStateGroup.h에서 가져옴)
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

// FTurretData 구조체 정의 (TurretStateGroup.h에서 가져옴)
USTRUCT(BlueprintType)
struct FTurretData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretData")
	FMaxCurrentData Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretData")
	float FireCoolTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretData")
	int32 ReloadAmmo = 100;
};
