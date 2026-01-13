// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JHS/GameControl/DataStruct.h"
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

UCLASS(BlueprintType)
class UEventOnChangeTurretAmmo : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnChangeTurretAmmo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UEventOnChangeTurretAmmo(FMaxCurrentData Ammo, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, Ammo(Ammo)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|TurretAmmo")
	FMaxCurrentData Ammo;
};
