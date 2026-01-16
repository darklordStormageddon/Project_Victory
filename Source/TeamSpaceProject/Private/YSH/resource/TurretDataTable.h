// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "JHS/GameControl/StateData/GameStateData.h"
#include "TurretDataTable.generated.h"

USTRUCT(BlueprintType)
struct FTurretInitState : public FTableRowBase
{
    GENERATED_BODY()

public:
    FTurretInitState() : bIsMainTurret(0), AmmoType("Bullet"), InitMaxMag(100), InitFireInterval(0.1f) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMainTurret;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AmmoType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InitMaxMag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InitFireInterval;
};

UCLASS()
class ATurretDataTable : public AActor
{
    GENERATED_BODY()
};

