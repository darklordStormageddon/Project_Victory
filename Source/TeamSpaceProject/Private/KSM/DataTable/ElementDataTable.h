// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "ElementDataTable.generated.h"

USTRUCT(BlueprintType)
struct FElementProperty : public FTableRowBase
{
    GENERATED_BODY()

public:
    FElementProperty() :ElementType(E_ELEMENT_TYPE::Aluminum), Price(0.0f) {}

    //원소 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_ELEMENT_TYPE ElementType;

    //가격
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Price;
};

UCLASS()
class AElementDataTable : public AActor
{
    GENERATED_BODY()
};

