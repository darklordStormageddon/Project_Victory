// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "CommonEventBase.generated.h"

/**
 * 모든 이벤트의 베이스 클래스
 */
UCLASS(BlueprintType)
class UCommonEventBase : public UObject
{
	GENERATED_BODY()

public:
	UCommonEventBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}
};

#pragma region SpaceShip
UCLASS(BlueprintType)
class UEventOnChangeSpaceShipData : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeSpaceShipData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeSpaceShipData(FSpaceShipData SpaceShipData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, SpaceShipDataData(SpaceShipData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|SpaceShipData")
	FSpaceShipData SpaceShipDataData;
};
#pragma endregion SpaceShip

#pragma region Player
UCLASS(BlueprintType)
class UEventOnChangePlayerRadiation : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangePlayerRadiation(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangePlayerRadiation(FPlayerStateData PlayerStateData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, PlayerStateData(PlayerStateData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|SpaceShipData")
	FPlayerStateData PlayerStateData;
};
#pragma endregion Player

#pragma region Turret
UCLASS(BlueprintType)
class UEventOnChangeTurretData : public UCommonEventBase
{
	GENERATED_BODY()
	
public:
	UEventOnChangeTurretData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeTurretData(FTurretData TurretData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, TurretData(TurretData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretData")
	FTurretData TurretData;
};

UCLASS(BlueprintType)
class UEventOnChangeTurretAmmo : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeTurretAmmo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeTurretAmmo(E_TURRET_POSITION TurretPosition, FMaxCurrentData Ammo, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, TurretPosition(TurretPosition)
		, Ammo(Ammo)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretAmmo")
	E_TURRET_POSITION TurretPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretAmmo")
	FMaxCurrentData Ammo;
};
#pragma endregion Turret

#pragma region Container
UCLASS(BlueprintType)
class UEventOnChangeElementData : public UCommonEventBase
{
	GENERATED_BODY()

public:
UEventOnChangeElementData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeElementData(E_ELEMENT_TYPE ElementType, int32 Amount, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, ElementType(ElementType)
		, Amount(Amount)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	E_ELEMENT_TYPE ElementType;

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	int32 Amount;
};
#pragma endregion Container