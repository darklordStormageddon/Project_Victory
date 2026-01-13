// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStruct.generated.h"

USTRUCT(BlueprintType)
struct FMaxCurrentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentValue;
};

UENUM(BlueprintType)
enum class E_SPACE_SHIP_DATA_TYPE : uint8
{
	HP = 0 UMETA(DisplayName = "HP"),
	Shield UMETA(DisplayName = "Shield"),
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
	FSpaceShipData Hp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FSpaceShipData Shield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FSpaceShipData Fuel;
};

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