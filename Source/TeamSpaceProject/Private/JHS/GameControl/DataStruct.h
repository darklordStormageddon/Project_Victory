// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStruct.generated.h"

UENUM(BlueprintType)
enum class E_DATA_TYPE : uint8
{
	HP = 0 UMETA(DisplayName = "HP"),
	Shield UMETA(DisplayName = "Shield"),
	Fuel UMETA(DisplayName = "Fuel"),
};

USTRUCT(BlueprintType)
struct FMaxCurrentData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	E_DATA_TYPE DataType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentValue;
};

USTRUCT(BlueprintType)
struct FSpaceShipData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FMaxCurrentData Hp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FMaxCurrentData Shield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	FMaxCurrentData Fuel;
};
