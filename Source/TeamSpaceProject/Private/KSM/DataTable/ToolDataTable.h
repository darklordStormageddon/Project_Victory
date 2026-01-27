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
    FToolProperty() :ToolType(E_COLLECT_TOOL_TYPE::Vacuum), Durability(0.0f), Damage(0.0f) {}

    //원소 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_COLLECT_TOOL_TYPE ToolType;

    //가격
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Durability;

    //Tex뒤에 올 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;
};

UCLASS()
class AToolDataTable : public AActor
{
    GENERATED_BODY()
};
