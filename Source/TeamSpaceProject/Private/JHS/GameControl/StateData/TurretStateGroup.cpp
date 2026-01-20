// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"
#include "YSH/resource/TurretDataTable.h"
#include "JHS/GameControl/CommonEnums.h"

// Sets default values for this component's properties
UTurretStateGroup::UTurretStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTurretStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTurretStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTurretStateGroup::InitializeTurretState(TObjectPtr<AJHSGameState> GameState, TArray<FAmmoData> AmmoDataArray)
{
	_gameState = GameState;

	_ammoDataMap.Empty();
	for (FAmmoData _ammoData : AmmoDataArray)
	{
		_ammoDataMap.Add(_ammoData.AmmoType, _ammoData);
	}

	_equipTurretMap.Empty();
	for (int32 i = 0; i < (int32)E_TURRET_POSITION::END; i++)
	{
		FTurretState _turretState;
		_turretState.TurretPosition = (E_TURRET_POSITION)i;
		_turretState._isEquipped = false;
		_equipTurretMap.Add(_turretState.TurretPosition, _turretState);
		UE_LOG(LogTemp, Warning, TEXT("%d"), i);
	}

	LoadTurretDataTable();
}

void UTurretStateGroup::UpdateTurretState()
{
	for (auto& _turretState : _equipTurretMap)
	{
		ExecuteTurretEvent(&_turretState.Value);
	}
}

bool UTurretStateGroup::TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType)
{
	FTurretState* _outTurretState = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretState))
	{
		UE_LOG(LogTemp, Error, TEXT("Not initialized turret position: %d"), (int32)TurretPosition);
		return false;
	}

	if (_outTurretState->_isEquipped && _outTurretState->TurretData.AmmoType == AmmoType)
	{
		UE_LOG(LogTemp, Error, TEXT("Same turret position and ammo type"));
		return false;
	}

	FTurretData* _outTurretData = nullptr;
	bool _isMainTurret = TurretPosition == E_TURRET_POSITION::Main;
	if (!TryGetTurretData(_isMainTurret, AmmoType, _outTurretData))
	{
		FString _isMainTurretString = _isMainTurret == true ? TEXT("True") : TEXT("False");
		UE_LOG(LogTemp, Error, TEXT("Failed to get turret data. IsMainTurret: %s, AmmoType: %d"), *_isMainTurretString, (int32)AmmoType);
		return false;
	}

	_outTurretState->TurretData = *_outTurretData;
	_outTurretState->_isEquipped = true;

	ExecuteTurretEvent(_outTurretState);
	return true;
}

bool UTurretStateGroup::TryGetTurretFireInterval(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime)
{
	FTurretState* _outTurretState = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretState))
	{
		UE_LOG(LogTemp, Error, TEXT("Not initialized turret position: %d"), (int32)TurretPosition);
		return false;
	}

	if (!_outTurretState->_isEquipped)
		return false;

	*OutFireCoolTime = _outTurretState->TurretData.FireInterval;
	return true;
}

bool UTurretStateGroup::TryFireTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretState* _outTurretState = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretState))
	{
		UE_LOG(LogTemp, Error, TEXT("Not initialized turret position: %d"), (int32)TurretPosition);
		return false;
	}

	if (!_outTurretState->_isEquipped)
		return false;

	if (_outTurretState->TurretData.Mag.CurrentValue <= 0)
		return false;

	ChangeTurretAmmo(_outTurretState, CONSUME_AMMO);
	return true;
}

bool UTurretStateGroup::TryReloadTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretState* _outTurretState = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretState))
	{
		UE_LOG(LogTemp, Error, TEXT("Not initialized turret position: %d"), (int32)TurretPosition);
		return false;
	}

	if (!_outTurretState->_isEquipped)
		return false;

	FAmmoData* _outAmmoData = nullptr;
	if (!TryGetAmmoData(_outTurretState->TurretData.AmmoType, _outAmmoData))
	{
		UE_LOG(LogTemp, Error, TEXT("Not found ammo data. AmmoType: %d"), (int32)_outTurretState->TurretData.AmmoType);
		return false;
	}

	ChangeTurretAmmo(_outTurretState, _outAmmoData->ReloadCapacity);
	return true;
}

void UTurretStateGroup::LoadTurretDataTable()
{
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
	for (const FName& RowName : _rowNames)
	{
		FTurretInitState* _turrerInfo = _turretDataTable->FindRow<FTurretInitState>(RowName, TEXT(""));
		if (_turrerInfo)
		{
			E_AMMO_TYPE _outAmmoType = E_AMMO_TYPE::NONE;
			if (!CommonEnums::TryGetAmmoType(_turrerInfo->AmmoType, _outAmmoType))
				return;

			FTurretData _newTurretData;
			_newTurretData.AmmoType = _outAmmoType;
			_newTurretData.Mag.MaxValue = _turrerInfo->InitMaxMag;
			_newTurretData.Mag.CurrentValue = _newTurretData.Mag.MaxValue;
			_newTurretData.FireInterval = _turrerInfo->InitFireInterval;

			_turretDataMap.Add(GetTurretKey(_turrerInfo->bIsMainTurret, _outAmmoType), _newTurretData);
		}
	}

	// 데이터 테이블이 비어있으면 실패
	if (_turretDataMap.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Turret Data Table is empty"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Turret Data Table loaded successfully. %d rows loaded"), _turretDataMap.Num());
	return;
}

int32 UTurretStateGroup::GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	return ((int32)IsMainTurret + 1) * HUNDRED + (int32)AmmoType;
}

bool UTurretStateGroup::TryGetAmmoData(E_AMMO_TYPE AmmoType, FAmmoData*& OutAmmoData)
{
	if (!_ammoDataMap.Contains(AmmoType))
		return false;

	OutAmmoData = _ammoDataMap.Find(AmmoType);
	return OutAmmoData != nullptr;
}

bool UTurretStateGroup::TryGetTurretData(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData)
{
	int32 _turretKey = GetTurretKey(IsMainTurret, AmmoType);
	if (!_turretDataMap.Contains(_turretKey))
		return false;

	OutTurretData = _turretDataMap.Find(_turretKey);
	return OutTurretData != nullptr;
}

bool UTurretStateGroup::TryGetEquipedTurret(E_TURRET_POSITION TurretPosition, FTurretState*& OutTurretState)
{
	if (!_equipTurretMap.Contains(TurretPosition))
		return false;

	OutTurretState = _equipTurretMap.Find(TurretPosition);
	if (OutTurretState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Not initialized turret state. TurretPosition: %d"), (int32)TurretPosition);
		return false;
	}

	return true;
}

void UTurretStateGroup::ChangeTurretAmmo(FTurretState* TurretState, int32 ChangeValue)
{
	TurretState->TurretData.Mag.CurrentValue += ChangeValue;
	if (TurretState->TurretData.Mag.CurrentValue > TurretState->TurretData.Mag.MaxValue)
	{
		TurretState->TurretData.Mag.CurrentValue = TurretState->TurretData.Mag.MaxValue;
	}
	if (TurretState->TurretData.Mag.CurrentValue < 0)
	{
		TurretState->TurretData.Mag.CurrentValue = 0;
	}

	ExecuteTurretEvent(TurretState);
}

void UTurretStateGroup::ExecuteTurretEvent(FTurretState* TurretState)
{
	UEventOnChangeTurretState* _event = NewObject<UEventOnChangeTurretState>(this);
	_event->TurretState = *TurretState;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretState>(_event);
}