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

	UEventOnChangeSpaceShipData(FSpaceShipData InSpaceShipData, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
		, SpaceShipData(InSpaceShipData)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Event|SpaceShipData")
	FSpaceShipData SpaceShipData;
};
