// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

// Sets default values for this component's properties
USpaceShipStateGroup::USpaceShipStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	_spaceShipState.Shield.DataType = E_SPACE_SHIP_DATA_TYPE::Shield;
	_spaceShipState.HP.DataType = E_SPACE_SHIP_DATA_TYPE::HP;
	_spaceShipState.Fuel.DataType = E_SPACE_SHIP_DATA_TYPE::Fuel;
}


// Called when the game starts
void USpaceShipStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpaceShipStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpaceShipStateGroup::InitializeSpaceShipState(TObjectPtr<AJHSGameState> GameState, FSpaceShipState InitSpaceShipState)
{
	_gameState = GameState;

	_spaceShipState.Shield.Data.Value = InitSpaceShipState.Shield.Data.Value;
	_spaceShipState.HP.Data.Value = InitSpaceShipState.HP.Data.Value;
	_spaceShipState.Fuel.Data.Value = InitSpaceShipState.Fuel.Data.Value;

	RepairSpaceShip();
}

void USpaceShipStateGroup::UpdateSpaceShipState()
{
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Data.Value.CurrentValue, _spaceShipState.Shield.Data.Value.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.HP, _spaceShipState.HP.Data.Value.CurrentValue, _spaceShipState.HP.Data.Value.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Data.Value.CurrentValue, _spaceShipState.Fuel.Data.Value.MaxValue);
}

void USpaceShipStateGroup::RepairSpaceShip()
{
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Data.Value.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.HP, _spaceShipState.HP.Data.Value.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Data.Value.MaxValue);
}

void USpaceShipStateGroup::DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue)
{
	switch (DataType)
	{
		case E_SPACE_SHIP_DATA_TYPE::Shield:
			ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Data.Value.CurrentValue - DecreaseValue);
			break;

		case E_SPACE_SHIP_DATA_TYPE::HP:
			ChangeSpaceShipData(&_spaceShipState.HP, _spaceShipState.HP.Data.Value.CurrentValue - DecreaseValue);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Fuel:
			ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Data.Value.CurrentValue - DecreaseValue);
			break;

		default:
			break;
	}
}

void USpaceShipStateGroup::RepairShield(float RepairShieldValue)
{
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Data.Value.CurrentValue + RepairShieldValue);
}

void USpaceShipStateGroup::ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue)
{
	ChangeSpaceShipData(OriginalData, CurrentValue, OriginalData->Data.Value.MaxValue);
}

void USpaceShipStateGroup::ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue, float MaxValue)
{
	// FMaxCurrentData 원본 데이터 참조
	FMaxCurrentData* _value = &(OriginalData->Data.Value);
	_value->CurrentValue = CurrentValue;
	_value->MaxValue = MaxValue;
	if (_value->CurrentValue > _value->MaxValue)
	{
		_value->CurrentValue = _value->MaxValue;
	}
	if (_value->CurrentValue < 0.0f)
	{
		_value->CurrentValue = 0.0f;
	}

	UEventOnChangeSpaceShipData* _event = NewObject<UEventOnChangeSpaceShipData>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("USpaceShipStateGroup: Failed to create UEventOnChangeSpaceShipData"));
		return;
	}

	_event->SpaceShipDataData = *OriginalData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeSpaceShipData>(_event);
}