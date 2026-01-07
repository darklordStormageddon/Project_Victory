// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JHS/GameControl/DataStruct.h"

#include "JHSGameState.generated.h"

/**
 * 
 */
UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	FSpaceShipData _spaceShipData;

public:
	FSpaceShipData GetSpaceShipMaxHP() { return _spaceShipData; }
};
