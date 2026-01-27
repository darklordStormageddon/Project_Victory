// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "ToolDataTable.generated.h"

UENUM(BlueprintType)
enum class E_TOOL_Type : uint8
{
    // ¾Ë·ç¹Ì´½, Ã¶, Æ¼Å¸´½, ´ÏÄÌ, ±¸¸®, ½Ç¸®ÄÜ, ±Ý, ¸®Æ¬, Åº¼Ò¼¶À¯
    Laser = 0 UMETA(DisplayName = "Laser"),
    Vacuum UMETA(DisplayName = "Vacuum"),
    Drill UMETA(DisplayName = "Drill"),
    NONE UMETA(DisplayName = "NONE"),
};


USTRUCT(BlueprintType)
struct FToolProperty : public FTableRowBase
{
    GENERATED_BODY()

public:
    FToolProperty() :ToolType(E_TOOL_Type::Laser), Durability(0.0f), Damage(0.0f) {}

    //¿ø¼Ò Á¾·ù
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    E_TOOL_Type ToolType;

    //°¡°Ý
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Durability;

    //TexµÚ¿¡ ¿Ã ÀÌ¸§
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;
};

UCLASS()
class AToolDataTable : public AActor
{
    GENERATED_BODY()
};
