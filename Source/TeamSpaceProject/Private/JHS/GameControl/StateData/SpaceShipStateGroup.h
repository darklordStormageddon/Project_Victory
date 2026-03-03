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

	UPROPERTY(ReplicatedUsing = "OnRep_SpaceShipDataArray")
	TArray<FSpaceShipData> _replicatedSpaceShipDataArray;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_SpaceShipDataArray();

	const float CONSUME_FUEL_VALUE = 1.0f;

	// Repair Shield
	float _repairDelay = 0.0f;

	float _repairShieldValue = 0.0f;

	FTimerHandle _timerHandleDamageDelay;

	FTimerHandle _timerHandleRepairShield;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeSpaceShipState(TObjectPtr<AJHSGameState> GameState, float RepairDelay, float RepairShieldValue);

	void UpdateSpaceShipState();

	TArray<FPurchaseData*> GetPurchaseDataArray();

#pragma region Health
public:
	void RepairSpaceShip();

	void TakeDamage(float Damage);

private:
	void StartRepairShield();

	void RepairShield();
#pragma endregion Health

#pragma region Feul
public:
	bool TryConsumeFuel();
#pragma endregion Feul

#pragma region Radiation
public:
	bool TryGetRadiationData(FPurchaseData*& OutRadiationData);
#pragma endregion Radiation

private:
	void LoadSpaceShipData();

	void DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue);

	void TryPurchaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType);

	void ChangeMaxData(FSpaceShipData* OriginalData, float MaxValue, bool IsRepairCurrentValue);

	void ChangCurrentData(FSpaceShipData* OriginalData, float CurrentValue);

	void ExecuteEventSpaceShipData(FSpaceShipData SpaceShipData);

	bool TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, FSpaceShipData*& OutSpaceShipData);

	void SyncSpaceShipDataToReplicated();

public:
	void ExecutePurchaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType);
};
