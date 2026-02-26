// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/ShopManager.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "PSJ/SpaceShipDataTable.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/CommonEnums.h"

// Sets default values for this component's properties
USpaceShipStateGroup::USpaceShipStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void USpaceShipStateGroup::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void USpaceShipStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USpaceShipStateGroup::InitializeSpaceShipState(TObjectPtr<AJHSGameState> GameState, float RepairDelay, float RepairShieldValue)
{
	_gameState = GameState;
	_repairDelay = RepairDelay;
	_repairShieldValue = RepairShieldValue;

	LoadSpaceShipData();
	RepairSpaceShip();
}

void USpaceShipStateGroup::UpdateSpaceShipState()
{
	FSpaceShipData* _outSpaceShipData = nullptr;
	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.CurrentValue);
	}

	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.CurrentValue);
	}

	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Fuel, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.CurrentValue);
	}
}

TArray<FPurchaseData*> USpaceShipStateGroup::GetPurchaseDataArray()
{
	TArray<FPurchaseData*> _purchaseDataArray;
	for (int8 i = 0; i < (int8)E_SPACE_SHIP_DATA_TYPE::NONE; i++)
	{
		E_SPACE_SHIP_DATA_TYPE _dataType = (E_SPACE_SHIP_DATA_TYPE)i;

		FSpaceShipData* _outSpaceShipData = nullptr;
		if (!TryGetSpaceShipData(_dataType, _outSpaceShipData))
			continue;

		FPurchaseData* _data = &_outSpaceShipData->Data;
		_data->OnPurchaseRequested.BindLambda([this, _dataType]() { TryPurchaseSpaceShipData(_dataType); });
		_purchaseDataArray.Add(_data);
	}

	return _purchaseDataArray;
}

#pragma region Health
void USpaceShipStateGroup::RepairSpaceShip()
{
	FSpaceShipData* _outSpaceShipData = nullptr;
	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.MaxValue);
	}

	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.MaxValue);
	}

	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Fuel, _outSpaceShipData))
	{
		ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.MaxValue);
	}
}

void USpaceShipStateGroup::TakeDamage(float Damage)
{
	// Shield
	FSpaceShipData* _outShieldData = nullptr;
	if (!TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, _outShieldData))
		return;

	FMaxCurrentData* _shieldData = &_outShieldData->Data.Value;
	float _leftDamage = Damage - _shieldData->CurrentValue;
	ChangCurrentData(_outShieldData, _shieldData->CurrentValue - Damage);

	if (_leftDamage <= 0.0f)
		return;

	// HP
	FSpaceShipData* _outHPData = nullptr;
	if (!TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, _outHPData))
		return;

	FMaxCurrentData* _hpData = &_outHPData->Data.Value;
	ChangCurrentData(_outHPData, _hpData->CurrentValue - _leftDamage);

	// Is died
	if (_hpData->CurrentValue <= 0.0f)
	{
		AJHSPlayerController* _outPlayerController = nullptr;
		if (!UStaticFunctionLibrary::TryGetPlayerController(_outPlayerController))
			return;

		APawn* _pawn = _outPlayerController->GetPawn();
		if (_pawn->HasAuthority())
		{
			AJHSGameMode* _outGameMode = nullptr;
			if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
				return;

			_outGameMode->EndStage(_pawn);
		}

		return;
	}

	// Repair shield timer
	StartRepairShield();
}

void USpaceShipStateGroup::StartRepairShield()
{
	GetWorld()->GetTimerManager().ClearTimer(_timerHandleDamageDelay);
	GetWorld()->GetTimerManager().ClearTimer(_timerHandleRepairShield);
	GetWorld()->GetTimerManager().SetTimer(_timerHandleRepairShield, this, &USpaceShipStateGroup::RepairShield, _repairDelay, false);
}

void USpaceShipStateGroup::RepairShield()
{
	float _deltaTime = UStaticFunctionLibrary::GetDeltaTime();

	FSpaceShipData* _outSpaceShipData = nullptr;
	if (TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, _outSpaceShipData))
	{
		FMaxCurrentData* _shieldData = &_outSpaceShipData->Data.Value;
		ChangCurrentData(_outSpaceShipData, _shieldData->CurrentValue + _deltaTime * _repairShieldValue);

		if (_shieldData->CurrentValue >= _shieldData->MaxValue)
			return;
	}

	if (_deltaTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(_timerHandleRepairShield, this, &USpaceShipStateGroup::RepairShield, _deltaTime, false);
	}
}
#pragma endregion Health

#pragma region Feul
bool USpaceShipStateGroup::TryConsumeFuel()
{
	FSpaceShipData* _outFuelData = nullptr;
	if (!TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Fuel, _outFuelData))
		return false;

	FMaxCurrentData* _fuelData = &_outFuelData->Data.Value;
	float _consumeValue = CONSUME_FUEL_VALUE * UStaticFunctionLibrary::GetDeltaTime();
	if (_fuelData->CurrentValue < _consumeValue)
		return false;

	ChangCurrentData(_outFuelData, _outFuelData->Data.Value.CurrentValue - _consumeValue);
	return true;
}
#pragma endregion Feul

#pragma region Radiation
bool USpaceShipStateGroup::TryGetRadiationData(FMaxCurrentData*& OutRadiationData)
{
	FSpaceShipData* _outRadiationData = nullptr;
	if (!TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Radiation, _outRadiationData))
		return false;

	OutRadiationData = &_outRadiationData->Data.Value;
	return OutRadiationData != nullptr;
}
#pragma endregion Radiation

void USpaceShipStateGroup::LoadSpaceShipData()
{
	_spaceShipDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.SPACESHIP_PATH));
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("USpaceShipStateGroup: Failed to load SpaceShip Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.SPACESHIP_PATH);
		return;
	}

	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FSpaceShipDataRow* _spaceShipDataRow = _turretDataTable->FindRow<FSpaceShipDataRow>(_rowName, TEXT(""));
		if (_spaceShipDataRow)
		{
			FSpaceShipData _newSpaceShipData;
			_newSpaceShipData.DataType = _spaceShipDataRow->SpaceShipDataType;
			TObjectPtr<UTexture2D> _outTexture = nullptr;
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_SPACE_SHIP_DATA_TYPE>(_newSpaceShipData.DataType);
			if (AJHSGameState::TryGetTextureFromPath(ConstantLibrary::Resource.Image.SPACESHIP_FOLDER_PATH, _fileName, _outTexture))
			{
				_newSpaceShipData.SpaceShipDataImage = _outTexture;
			}

			_newSpaceShipData.Data = AJHSGameState::ParseFromDataRow(_newSpaceShipData.SpaceShipDataImage, _spaceShipDataRow->Data);

			_spaceShipDataMap.Add(_newSpaceShipData.DataType, _newSpaceShipData);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SpaceShip Data Table loaded successfully. %d rows loaded"), _spaceShipDataMap.Num());
}

void USpaceShipStateGroup::DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue)
{
	FSpaceShipData* _outSpaceShipData = nullptr;
	if (!TryGetSpaceShipData(DataType, _outSpaceShipData))
		return;

	ChangCurrentData(_outSpaceShipData, _outSpaceShipData->Data.Value.CurrentValue - DecreaseValue);
}

void USpaceShipStateGroup::TryPurchaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType)
{
	FSpaceShipData* _outSpaceShipData = nullptr;
	if (!TryGetSpaceShipData(DataType, _outSpaceShipData) || _gameState == nullptr)
		return;

	TObjectPtr<UShopManager> _shopManager = _gameState->GetShopManager();
	if (_shopManager == nullptr || !_shopManager->TryPurchase(&_outSpaceShipData->Data))
		return;

	ChangeMaxData(_outSpaceShipData, _outSpaceShipData->Data.Value.MaxValue, true);
}

void USpaceShipStateGroup::ChangeMaxData(FSpaceShipData* OriginalData, float MaxValue, bool IsRepairCurrentValue)
{
	FMaxCurrentData* _value = &(OriginalData->Data.Value);
	_value->MaxValue = MaxValue;
	if (_value->MaxValue < 0.0f)
	{
		_value->MaxValue = 0.0f;
	}

	if (IsRepairCurrentValue)
	{
		_value->CurrentValue = _value->MaxValue;
	}
	else if (_value->CurrentValue > _value->MaxValue)
	{
		_value->CurrentValue = _value->MaxValue;
	}

	ExecuteEventSpaceShipData(*OriginalData);
}

void USpaceShipStateGroup::ChangCurrentData(FSpaceShipData* OriginalData, float CurrentValue)
{
	FMaxCurrentData* _value = &(OriginalData->Data.Value);
	_value->CurrentValue = CurrentValue;
	if (_value->CurrentValue > _value->MaxValue)
	{
		_value->CurrentValue = _value->MaxValue;
	}
	if (_value->CurrentValue < 0.0f)
	{
		_value->CurrentValue = 0.0f;
	}

	ExecuteEventSpaceShipData(*OriginalData);
}

void USpaceShipStateGroup::ExecuteEventSpaceShipData(FSpaceShipData SpaceShipData)
{
	UEventOnChangeSpaceShipData* _event = NewObject<UEventOnChangeSpaceShipData>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("USpaceShipStateGroup: Failed to create UEventOnChangeSpaceShipData"));
		return;
	}

	_event->SpaceShipDataData = SpaceShipData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeSpaceShipData>(_event);
}

bool USpaceShipStateGroup::TryGetSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, FSpaceShipData*& OutSpaceShipData)
{
	if (!_spaceShipDataMap.Contains(DataType))
	{
		FString _spaceShipDataType = CommonEnums::GetEnum2FString<E_SPACE_SHIP_DATA_TYPE>(DataType);
		UE_LOG(LogTemp, Error, TEXT("USpaceShipStateGroup: Invalid. SpaceShipDataType: [%s]"), *_spaceShipDataType);
		return false;
	}

	OutSpaceShipData = _spaceShipDataMap.Find(DataType);
	return OutSpaceShipData != nullptr;
}