// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretDataTable.generated.h"

USTRUCT(BlueprintType)
struct FTurretInitState : public FTableRowBase
{
    GENERATED_BODY()

public:
    FTurretInitState() :TurretType(E_TURRET_TYPE::NONE), bIsMainTurret(0), AmmoType(E_AMMO_TYPE::NONE), Description("Description"), Price(0), Mag(), FireInterval() {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_TURRET_TYPE TurretType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMainTurret;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_AMMO_TYPE AmmoType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Price;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat Mag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat FireInterval;
};

USTRUCT(BlueprintType)
struct FAmmoInitState : public FTableRowBase
{
    GENERATED_BODY()

public:
    FAmmoInitState() :AmmoType(E_AMMO_TYPE::NONE), Price(0), ReloadCapacity(), AmmoDamage() {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_AMMO_TYPE AmmoType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Price;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat ReloadCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat AmmoDamage;
};

UCLASS()
class ATurretDataTable : public AActor
{
    GENERATED_BODY()
};

