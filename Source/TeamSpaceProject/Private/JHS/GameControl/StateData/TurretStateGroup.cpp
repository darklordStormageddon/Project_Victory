// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "YSH/resource/TurretDataTable.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/Turret/TurretStand.h"
#include "YSH/TurretChair.h"

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

	// _turretChair
	TArray<TObjectPtr<AActor>> _actorArray;
	_turretChair = nullptr;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurretChair::StaticClass(), _actorArray);
	for (TObjectPtr<AActor> _turretChairActor : _actorArray)
	{
		TObjectPtr<ATurretChair> _castedTurretChair = Cast<ATurretChair>(_turretChairActor);
		if (_castedTurretChair != nullptr)
		{
			_turretChair = _castedTurretChair;
		}
	}

	if (_turretChair == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: TurretChair not found."));
		return;
	}

	// 레벨에서 모든 ATurretStand 찾아서 설정
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurretStand::StaticClass(), _actorArray);
	for (TObjectPtr<AActor> _turretStand : _actorArray)
	{
		TObjectPtr<ATurretStand> _turretStandActor = Cast<ATurretStand>(_turretStand);
		if (!_turretStandActor)
		{
			UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: Failed to cast TurretStand. %s"), *_turretStand->GetName());
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

void UTurretStateGroup::TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType)
{
	// 서버/클라이언트 모두에서 호출 가능
	ServerEquipTurret(TurretPosition, AmmoType);
}

void UTurretStateGroup::ServerEquipTurret_Implementation(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType)
{
	// 서버에서만 실행: 데이터 검증 및 장착 가능 여부 체크
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to get turret stand"));
		return;
	}

	// 장착 가능 여부 체크
	if (!_outTurretStand->CanEquipTurret())
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretStateGroup: Turret already equipped at [%s]"), *CommonEnums::GetEnum2FString<E_TURRET_POSITION>(TurretPosition));
		return;
	}

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, AmmoType, _outTurretData))
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to get turret data"));
		return;
	}

	// 터렛 BP 클래스 로드 및 스폰
	FString _fileName = "BP_" + CommonEnums::GetEnum2FString<E_TURRET_TYPE>(_outTurretData->TurretType);
	FString _turretBPPath = ConstantLibrary::Resource.TurretBP.TURRET_BP_FOLDER_PATH + _fileName + "." + _fileName + "_C";
	TSubclassOf<AActor> _turretClass = LoadClass<AActor>(nullptr, *_turretBPPath);
	if (_turretClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load turret BP class: %s"), *_turretBPPath);
		return;
	}

	// 터렛 액터 스폰 (서버에서만)
	FVector _spawnLocation = _outTurretStand->GetActorLocation() + _outTurretStand->GetActorUpVector() * 1.0f;
	FActorSpawnParameters _spawnParams;
	_spawnParams.Owner = _outTurretStand;
	AActor* _spawnedTurret = GetWorld()->SpawnActor<AActor>(_turretClass, _spawnLocation, _outTurretStand->GetActorRotation(), _spawnParams);
	if (_spawnedTurret == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to spawn turret: %s"), *_turretBPPath);
		return;
	}

	// 모든 클라이언트(서버 포함)에 터렛 장착 명령 (스폰된 터렛 전달)
	MulticastEquipTurret(TurretPosition, AmmoType, _spawnedTurret);
}

void UTurretStateGroup::MulticastEquipTurret_Implementation(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType, AActor* SpawnedTurret)
{
	// 모든 클라이언트(서버 포함)에서 실행: 터렛 장착
	if (SpawnedTurret == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: SpawnedTurret is null"));
		return;
	}

	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(TurretPosition, _outTurretStand))
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to get turret stand"));
		return;
	}

	// 터렛 장착
	bool _isEquiped = _outTurretStand->TryEquipTurret(SpawnedTurret, AmmoType);
	if (TurretPosition == E_TURRET_POSITION::Main && _isEquiped)
	{
		_turretChair->SetTargetPawn(Cast<APawn>(SpawnedTurret));
	}
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

	*OutFireCoolTime = _outTurretData->FireInterval.Value.MaxValue;
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

	if (_outTurretData->Mag.Value.CurrentValue <= 0)
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

	ChangeTurretAmmo(TurretPosition, _equipedAmmoType, _outAmmoData->ReloadCapacity.Value.MaxValue);
	return true;
}

void UTurretStateGroup::LoadTurretDataTable()
{
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load Turret Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FTurretInitState* _turrerInfo = _turretDataTable->FindRow<FTurretInitState>(_rowName, TEXT(""));
		if (_turrerInfo)
		{
			FTurretData _newTurretData;
			// 터렛 정보
			_newTurretData.TurretType = _turrerInfo->TurretType;
			_newTurretData.AmmoType = _turrerInfo->AmmoType;
			TObjectPtr<UTexture2D> _outTexture = nullptr;
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_TURRET_TYPE>(_newTurretData.TurretType);
			if (AJHSGameState::TryGetTextureFromPath(ConstantLibrary::Resource.Image.TURRET_FOLDER_PATH, _fileName, _outTexture))
			{
				_newTurretData.TurretImage = _outTexture;
			}

			// 가격
			FPurchaseData _price;
			_price.Image = _newTurretData.TurretImage;
			_price.Description = FString::Printf(TEXT("%s 구매"), *_turrerInfo->Description);
			_price.Level.MaxValue = 1;
			_price.Level.CurrentValue = 0;
			_price.PurchaseDollar = _turrerInfo->Price;
			_newTurretData.Price = _price;

			// 탄약
			_newTurretData.Mag = AJHSGameState::ParseFromDataRow(_newTurretData.TurretImage, _turrerInfo->Mag);

			// 공격 속도
			_newTurretData.FireInterval = AJHSGameState::ParseFromDataRow(_newTurretData.TurretImage, _turrerInfo->FireInterval);

			_turretDataMap.Add(GetTurretKey(_turrerInfo->bIsMainTurret, _newTurretData.AmmoType), _newTurretData);
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

	FMaxCurrentData* _value = &(_outTurretData->Mag.Value);
	_value->CurrentValue += ChangeValue;
	if (_value->CurrentValue > _value->MaxValue)
	{
		_value->CurrentValue = _value->MaxValue;
	}
	if (_value->CurrentValue < 0)
	{
		_value->CurrentValue = 0;
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