// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "SpaceShipStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USpaceShipStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpaceShipStateGroup();

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

protected:
	UPROPERTY(EditAnywhere, Category = "SpaceShipStateGroup")
	FSpaceShipState _spaceShipState;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeSpaceShipState(TObjectPtr<AJHSGameState> GameState, FSpaceShipState InitSpaceShipState);

	void UpdateSpaceShipState();

	void RepairSpaceShip();

	void DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue);

	void RepairShield(float RepairShieldValue);

private:
	void ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue);

	void ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue, float MaxValue);
};
