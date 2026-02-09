// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "SpaceShipDataTable.generated.h"

USTRUCT(BlueprintType)
struct FSpaceShipDataRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FSpaceShipDataRow() :SpaceShipDataType(E_SPACE_SHIP_DATA_TYPE::NONE), Data() {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_SPACE_SHIP_DATA_TYPE SpaceShipDataType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat Data;
};

UCLASS()
class ASpaceShipDataTable : public AActor
{
	GENERATED_BODY()
};
