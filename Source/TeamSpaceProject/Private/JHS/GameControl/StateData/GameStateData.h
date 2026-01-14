// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameStateData.generated.h"

USTRUCT(BlueprintType)
struct FMaxCurrentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentValue;
};
