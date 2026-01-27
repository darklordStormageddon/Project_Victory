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

	UEventOnChangeTurretData(E_TURRET_POSITION TurretPosition, FTurretData TurretData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, TurretPosition(TurretPosition)
		, TurretData(TurretData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretData")
	E_TURRET_POSITION TurretPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretData")
	FTurretData TurretData;
};
#pragma endregion Turret

#pragma region Collect
UCLASS(BlueprintType)
class UEventOnCollectToolDurability : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnCollectToolDurability(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	UEventOnCollectToolDurability(FCollectToolData CollectToolData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, CollectToolData(CollectToolData)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Event|Collect")
	FCollectToolData CollectToolData;
};
#pragma endregion Collect


#pragma region Container
UCLASS(BlueprintType)
class UEventOnChangeElementData : public UCommonEventBase
{
	GENERATED_BODY()

public:
UEventOnChangeElementData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeElementData(FElementData ElementData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, ElementData(ElementData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	FElementData ElementData;
};
#pragma endregion Container