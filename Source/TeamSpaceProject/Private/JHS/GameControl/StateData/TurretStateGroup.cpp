// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/Turret/TurretStand.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
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

void UTurretStateGroup::InitializeTurretState(TObjectPtr<AJHSGameState> GameState)
{
	_gameState = GameState;

	// 레벨에서 모든 ATurretStand 찾아서 설정
	TArray<TObjectPtr<AActor>> _turretStandArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurretStand::StaticClass(), _turretStandArray);
	for (TObjectPtr<AActor> _turretStand : _turretStandArray)
	{
		TObjectPtr<ATurretStand> _turretStandActor = Cast<ATurretStand>(_turretStand);
		if (!_turretStandActor)
		{
			UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: TurretStand not found. %s"), *_turretStand->GetName());
			continue;
		}

		_turretStandMap.Add(_turretStandActor->GetTurretPosition(), _turretStandActor);
		_turretStandActor->InitializeTurretStand(this);
	}

	if (_turretStandMap.Num() < (int32)E_TURRET_POSITION::END)
	{
		UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: TurretStand not found. %d"), _turretStandMap.Num());
	}

	LoadTurretDataTable();
}

void UTurretStateGroup::UpdateTurretState()
{
	for (auto& _turretStand : _turretStandMap)
	{
		E_TURRET_POSITION _turretPosition = _turretStand.Value->GetTurretPosition();
		E_AMMO_TYPE _ammoType = _turretStand.Value->GetAmmoType();
		if (_ammoType == E_AMMO_TYPE::NONE)
			continue;

		FTurretData* _outTurretData = nullptr;
		if (!TryGetTurretData(_turretPosition, _ammoType, _outTurretData))
			continue;

		ExecuteTurretEvent(_turretPosition, *_outTurretData);
	}
}

void UTurretStateGroup::SetInfiniteMagMode(bool IsInfiniteMagMode)
{
	_isInfiniteMagMode = IsInfiniteMagMode;
}

bool UTurretStateGroup::TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, TObjectPtr<AActor> Turret)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
		return false;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, AmmoType, _outTurretData))
		return false;

	if (Turret == nullptr)
	{
		// 터렛 BP 클래스 로드
		FString _fileName = _outTurretData->TurretBPName;
		FString _turretBPPath = ConstantLibrary::Resource.TurretBP.TURRET_BP_FOLDER_PATH + _fileName + "." + _fileName + "_C";
		TSubclassOf<AActor> _turretClass = LoadClass<AActor>(nullptr, *_turretBPPath);
		if (_turretClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load turret BP class: %s"), *_turretBPPath);
			return false;
		}

		// 터렛 액터 스폰
		FActorSpawnParameters _spawnParams;
		_spawnParams.Owner = _outTurretStand;
		Turret = GetWorld()->SpawnActor<AActor>(_turretClass, _outTurretStand->GetActorTransform(), _spawnParams);
		if (Turret == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to spawn turret: %s"), *_turretBPPath);
			return false;
		}
	}
	
	return _outTurretStand->TryEquipTurret(Turret, AmmoType);
}

bool UTurretStateGroup::TryGetTurretFireInterval(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
		return false;

	E_AMMO_TYPE _equipedAmmoType = _outTurretStand->GetAmmoType();
	if (_equipedAmmoType == E_AMMO_TYPE::NONE)
		return false;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, _equipedAmmoType, _outTurretData))
		return false;

	*OutFireCoolTime = _outTurretData->FireInterval;
	return true;
}

bool UTurretStateGroup::TryFireTurret(E_TURRET_POSITION TurretPosition)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
		return false;

	E_AMMO_TYPE _equipedAmmoType = _outTurretStand->GetAmmoType();
	if (_equipedAmmoType == E_AMMO_TYPE::NONE)
		return false;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, _equipedAmmoType, _outTurretData))
		return false;

	if (_outTurretData->Mag.CurrentValue <= 0)
	{
		if (!_isInfiniteMagMode)
			return false;

		if (!TryReloadTurret(TurretPosition))
			return false;
	}

	ChangeTurretAmmo(TurretPosition, _equipedAmmoType, CONSUME_AMMO);
	return true;
}

bool UTurretStateGroup::TryReloadTurret(E_TURRET_POSITION TurretPosition)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
		return false;

	E_AMMO_TYPE _equipedAmmoType = _outTurretStand->GetAmmoType();
	if (_equipedAmmoType == E_AMMO_TYPE::NONE)
		return false;

	FAmmoData* _outAmmoData = nullptr;
	if (!_gameState->GetContainerStateGroup()->TryGetAmmoData(_equipedAmmoType, _outAmmoData))
		return false;

	ChangeTurretAmmo(TurretPosition, _equipedAmmoType, _outAmmoData->ReloadCapacity);
	return true;
}

void UTurretStateGroup::LoadTurretDataTable()
{
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FTurretInitState* _turrerInfo = _turretDataTable->FindRow<FTurretInitState>(_rowName, TEXT(""));
		if (_turrerInfo)
		{
			E_AMMO_TYPE _outAmmoType = E_AMMO_TYPE::NONE;
			if (!CommonEnums::TryGetAmmoType(_turrerInfo->AmmoType, _outAmmoType))
				return;

			FTurretData _newTurretData;
			_newTurretData.TurretBPName = _turrerInfo->TurretName;
			_newTurretData.AmmoType = _outAmmoType;
			_newTurretData.Mag.MaxValue = _turrerInfo->InitMaxMag;
			_newTurretData.Mag.CurrentValue = _newTurretData.Mag.MaxValue;
			_newTurretData.FireInterval = _turrerInfo->InitFireInterval;

			_turretDataMap.Add(GetTurretKey(_turrerInfo->bIsMainTurret, _outAmmoType), _newTurretData);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Turret Data Table loaded successfully. %d rows loaded"), _turretDataMap.Num());
}

int32 UTurretStateGroup::GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	return ((int32)IsMainTurret + 1) * HUNDRED + (int32)AmmoType;
}

bool UTurretStateGroup::TryGetTurretData(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData)
{
	bool _isMainTurret = TurretPosition == E_TURRET_POSITION::Main;
	int32 _turretKey = GetTurretKey(_isMainTurret, AmmoType);
	if (!_turretDataMap.Contains(_turretKey))
		return false;

	OutTurretData = _turretDataMap.Find(_turretKey);
	return OutTurretData != nullptr;
}

bool UTurretStateGroup::TryGetTurretStand(E_TURRET_POSITION TurretPosition, TObjectPtr<ATurretStand>& OutTurretStand)
{
	if (!_turretStandMap.Contains(TurretPosition))
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Not initialized turret state. TurretPosition: %d"), (int32)TurretPosition);
		return false;
	}

	OutTurretStand = _turretStandMap.FindRef(TurretPosition);
	return OutTurretStand != nullptr;
}

void UTurretStateGroup::ChangeTurretAmmo(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, int32 ChangeValue)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, AmmoType, _outTurretData))
		return;

	_outTurretData->Mag.CurrentValue += ChangeValue;
	if (_outTurretData->Mag.CurrentValue > _outTurretData->Mag.MaxValue)
	{
		_outTurretData->Mag.CurrentValue = _outTurretData->Mag.MaxValue;
	}
	if (_outTurretData->Mag.CurrentValue < 0)
	{
		_outTurretData->Mag.CurrentValue = 0;
	}

	ExecuteTurretEvent(TurretPosition, *_outTurretData);
}

void UTurretStateGroup::ExecuteTurretEvent(E_TURRET_POSITION TurretPosition, FTurretData TurretData)
{
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	_event->TurretPosition = TurretPosition;
	_event->TurretData = TurretData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
}