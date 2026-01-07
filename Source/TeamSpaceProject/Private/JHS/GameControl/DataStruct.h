// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStruct.generated.h"

USTRUCT(BlueprintType)
struct FSpaceShipData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	float CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	float MaxShield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceShipData")
	float CurrentShield;
};
