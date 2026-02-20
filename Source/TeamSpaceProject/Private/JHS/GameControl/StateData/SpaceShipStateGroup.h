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

	UPROPERTY()
	TMap<E_SPACE_SHIP_DATA_TYPE, FSpaceShipData> _spaceShipDataMap;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeSpaceShipState(TObjectPtr<AJHSGameState> GameState);

	void UpdateSpaceShipState();

	TArray<FPurchaseData*> GetPurchaseDataArray();

	void TryPurchaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType);

	void RepairSpaceShip();

	void DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue);

	void RepairShield(float RepairShieldValue);

private:
	void LoadSpaceShipData();

	void ChangCurrentData(FSpaceShipData* OriginalData, float CurrentValue);

	void ChangeMaxData(FSpaceShipData* OriginalData, float MaxValue, bool IsRepairCurrentValue);

	void ExecuteEventSpaceShipData(FSpaceShipData SpaceShipData);

	bool TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, FSpaceShipData*& OutSpaceShipData);
};
