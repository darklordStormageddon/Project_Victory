// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "ToolDataTable.generated.h"

USTRUCT(BlueprintType)
struct FToolProperty : public FTableRowBase
{
    GENERATED_BODY()

public:
    FToolProperty() :ToolType(E_COLLECT_TOOL_TYPE::Vacuum), Durability(), Damage() {}

    // 도구 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_COLLECT_TOOL_TYPE ToolType;

    // 내구도
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat Durability;

    // 도구 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPurchaseDataFormat Damage;
};

UCLASS()
class AToolDataTable : public AActor
{
    GENERATED_BODY()
};
