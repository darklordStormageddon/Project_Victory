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

#pragma region Game Control
UCLASS(BlueprintType)
class UEventOnStartStage : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnStartStage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	UEventOnStartStage(int32 State, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, State(State)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Event|GameControl")
	int32 State;
};

UCLASS(BlueprintType)
class UEventOnEndStage : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnEndStage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Event|GameControl")
	int32 State = 0;
};
#pragma endregion Game Control

#pragma region SpaceShip
UCLASS(BlueprintType)
class UEventOnChangeSpaceShipData : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeSpaceShipData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
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

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretData")
	E_TURRET_POSITION TurretPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretData")
	FTurretData TurretData;
};
#pragma endregion Turret

#pragma region Collect
UCLASS(BlueprintType)
class UEventOnChangeToolDurability : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeToolDurability(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Event|Collect")
	FCollectToolData CollectToolData;
};

UCLASS(BlueprintType)
class UEventOnChangeTool : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeTool(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Event|Collect")
	E_COLLECT_TOOL_TYPE PrevToolType;

	UPROPERTY(BlueprintReadOnly, Category = "Event|Collect")
	E_COLLECT_TOOL_TYPE NextToolType;
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

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	FElementData ElementData;

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	int32 CumulativePrice = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	int32 OwnedDollar = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Event|Container")
	int32 GoalDollar = 0;
};
#pragma endregion Container