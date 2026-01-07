// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JHS/GameControl/DataStruct.h"

#include "JHSGameState.generated.h"

class UEventManager;

UCLASS()
class AJHSGameState : public AGameState
{
	GENERATED_BODY()

private:
	TObjectPtr<UEventManager> _cachedEventManager = nullptr;
	
protected:
	UPROPERTY(EditAnywhere, Category = "GameMode|Game Data")
	FSpaceShipData _spaceShipData;

public:
	FSpaceShipData GetSpaceShipData() { return _spaceShipData; }

private:
	TObjectPtr<UEventManager> GetEventManager();

public:
	void ChangeSpaceShipData(FSpaceShipData SpaceShipData);
};
