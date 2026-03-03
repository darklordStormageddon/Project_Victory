// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "YSH/resource/TurretDataTable.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/GameControl/ShopManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/Turret/TurretStand.h"
#include "YSH/TurretChair.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTurretStateGroup::UTurretStateGroup()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UTurretStateGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTurretStateGroup, _replicatedTurretDataArray);
	DOREPLIFETIME(UTurretStateGroup, _replicatedAmmoDataArray);
	DOREPLIFETIME(UTurretStateGroup, _mainTurretType);
}

void UTurretStateGroup::OnRep_TurretDataArray()
{
	_turretDataMap.Empty();
	for (const FTurretData& _data : _replicatedTurretDataArray)
		_turretDataMap.Add(GetTurretKey(_data.IsMainTurret, _data.AmmoType), _data);
}

void UTurretStateGroup::OnRep_AmmoDataArray()
{
	_ammoDataMap.Empty();
	for (const FAmmoData& _data : _replicatedAmmoDataArray)
		_ammoDataMap.Add(_data.AmmoType, _data);
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

	LoadTurretDataTable();

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

		_turretStandMap.Add(_turretStandActor->GetStandType(), _turretStandActor);
		_turretStandActor->InitializeTurretStand(this);
	}

	if (_turretStandMap.Num() < (int32)E_AMMO_TYPE::NONE + 1)
	{
		UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: TurretStand not found. %d"), _turretStandMap.Num());
	}
}

void UTurretStateGroup::UpdateTurretState()
{
	for (auto& _element : _turretStandMap)
	{
		TObjectPtr<ATurretStand> _turretStand = _element.Value;
		E_AMMO_TYPE _standType = _element.Key;
		const bool& _isMain = _standType == E_AMMO_TYPE::NONE;
		if (_isMain)
		{
			_standType = _turretStand->GetTurretType();
		}

		if (_standType == E_AMMO_TYPE::NONE)
			continue;

		FTurretData* _outTurretData = nullptr;
		if (!TryGetTurretData(_isMain, _standType, _outTurretData))
			continue;

		ExecuteTurretEvent(_isMain, _standType, *_outTurretData);
	}
}

TArray<FPurchaseData*> UTurretStateGroup::GetTurretPurchaseDataArray()
{
	TArray<FPurchaseData*> _purchaseDataArray;

	// 메인 터렛
	TArray<FPurchaseData*> _turretPurchaseDataArray ;
	if (_mainTurretType != E_AMMO_TYPE::NONE)
	{
		_turretPurchaseDataArray = GetTurretPurchaseDataArray(true, _mainTurretType);
		for (auto& _purchaseData : _turretPurchaseDataArray)
		{
			_purchaseDataArray.Add(_purchaseData);
		}
	}

	// Auto
	for (int32 i = 0; i < (int32)E_AMMO_TYPE::NONE; i++)
	{
		E_AMMO_TYPE _ammoType = (E_AMMO_TYPE)i;
		_turretPurchaseDataArray = GetTurretPurchaseDataArray(false, _ammoType);
		for (auto& _purchaseData : _turretPurchaseDataArray)
		{
			_purchaseDataArray.Add(_purchaseData);
		}
	}

	return _purchaseDataArray;
}

TArray<FPurchaseData*> UTurretStateGroup::GetTurretPurchaseDataArray(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	TArray<FPurchaseData*> _purchaseDataArray;

	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return _purchaseDataArray;

	const bool _isTurretEmpty = _outTurretStand->CanEquipTurret();

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
		return _purchaseDataArray;

	for (int32 _fieldIndex = 0; _fieldIndex < 3; _fieldIndex++)
	{
		FPurchaseData* _data = _fieldIndex == 0 ? &_outTurretData->Price : (_fieldIndex == 1 ? &_outTurretData->Mag : &_outTurretData->FireInterval);

		// 구매 가능
		if (_fieldIndex == 0)
		{
			if (_data->IsPurchaseable)
			{
				_data->IsPurchaseable = _isTurretEmpty;
			}
		}
		else
		{
			_data->IsPurchaseable = !_isTurretEmpty;
		}
		_data->OnPurchaseRequested.BindLambda([this, IsMainTurret, AmmoType, _fieldIndex]()
		{
			TryPurchaseTurret(IsMainTurret, AmmoType, _fieldIndex);
		});

		_purchaseDataArray.Add(_data);
	}

	return _purchaseDataArray;
}

TArray<FPurchaseData*> UTurretStateGroup::GetAmmoPurchaseDataArray()
{
	TArray<FPurchaseData*> _purchaseDataArray;
	for (int8 i = 0; i < (int8)E_AMMO_TYPE::NONE; i++)
	{
		E_AMMO_TYPE _ammoType = (E_AMMO_TYPE)i;

		FAmmoData* _outAmmoData = nullptr;
		if (!TryGetAmmoData(_ammoType, _outAmmoData))
			continue;

		for (int32 _fieldIndex = 0; _fieldIndex < 3; _fieldIndex++)
		{
			FPurchaseData* _data = _fieldIndex == 0 ? &_outAmmoData->Price : (_fieldIndex == 1 ? &_outAmmoData->ReloadCapacity : &_outAmmoData->AmmoDamage);
			_data->OnPurchaseRequested.BindLambda([this, _ammoType, _fieldIndex]() { TryPurchaseAmmo(_ammoType, _fieldIndex); });
			_purchaseDataArray.Add(_data);
		}
	}

	return _purchaseDataArray;
}

void UTurretStateGroup::TryPurchaseTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ExecutePurchaseTurret(IsMainTurret, AmmoType, FieldIndex);
	}
	else
	{
		APlayerController* _playerController = GetWorld()->GetFirstPlayerController();
		AJHSPlayerController* _jhsPlayerController = Cast<AJHSPlayerController>(_playerController);
		if (_jhsPlayerController == nullptr)
			return;

		_jhsPlayerController->ServerRequestPurchaseTurret(IsMainTurret, AmmoType, FieldIndex);
	}
}

void UTurretStateGroup::ExecutePurchaseTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData) || _gameState == nullptr)
		return;

	FPurchaseData* _data = FieldIndex == 0 ? &_outTurretData->Price : (FieldIndex == 1 ? &_outTurretData->Mag : &_outTurretData->FireInterval);
	TObjectPtr<UShopManager> _shopManager = _gameState->GetShopManager();
	if (_shopManager == nullptr || !_shopManager->TryPurchase(_data))
		return;

	if (FieldIndex == 0)
	{
		TryEquipTurret(IsMainTurret, AmmoType);
	}

	ExecuteTurretEvent(IsMainTurret, AmmoType, *_outTurretData);
	SyncTurretStateToReplicated();
}

void UTurretStateGroup::TryPurchaseAmmo(E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ExecutePurchaseAmmo(AmmoType, FieldIndex);
	}
	else
	{
		APlayerController* _playerController = GetWorld()->GetFirstPlayerController();
		AJHSPlayerController* _jhsPlayerController = Cast<AJHSPlayerController>(_playerController);
		if (_jhsPlayerController == nullptr)
			return;

		_jhsPlayerController->ServerRequestPurchaseAmmo(AmmoType, FieldIndex);
	}
}

void UTurretStateGroup::ExecutePurchaseAmmo(E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	FAmmoData* _outAmmoData = nullptr;
	if (!TryGetAmmoData(AmmoType, _outAmmoData) || _gameState == nullptr)
		return;

	FPurchaseData* _data = FieldIndex == 0 ? &_outAmmoData->Price : (FieldIndex == 1 ? &_outAmmoData->ReloadCapacity : &_outAmmoData->AmmoDamage);
	TObjectPtr<UShopManager> _shopManager = _gameState->GetShopManager();
	if (_shopManager == nullptr || !_shopManager->TryPurchase(_data))
		return;
	SyncTurretStateToReplicated();
}

void UTurretStateGroup::SetStartSettings(bool IsInfiniteMagMode, E_AMMO_TYPE MainTurretType, bool IsStartEquipMainTurret, TArray<E_AMMO_TYPE> _startEquipAutoTurretArray)
{
	_isInfiniteMagMode = IsInfiniteMagMode;

	_mainTurretType = MainTurretType;
	if (_mainTurretType == E_AMMO_TYPE::NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("Main turret type is NONE"));
		return;
	}

	if (IsStartEquipMainTurret)
	{
		TryEquipTurret(true, _mainTurretType);
	}

	for (auto& _turretType : _startEquipAutoTurretArray)
	{
		TryEquipTurret(false, _turretType);
	}
}

bool UTurretStateGroup::TryGetTurretFireInterval(bool IsMainTurret, E_AMMO_TYPE AmmoType, float* OutFireCoolTime)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return false;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
		return false;

	*OutFireCoolTime = _outTurretData->FireInterval.Value.MaxValue;
	return true;
}

bool UTurretStateGroup::TryFireTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return false;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
		return false;

	if (_outTurretData->Mag.Value.CurrentValue <= 0)
	{
		if (!_isInfiniteMagMode)
			return false;

		if (!TryReloadTurret(IsMainTurret, AmmoType))
			return false;
	}

	ChangeTurretAmmo(IsMainTurret, AmmoType, CONSUME_AMMO);
	return true;
}

bool UTurretStateGroup::TryReloadTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return false;

	FAmmoData* _outAmmoData = nullptr;
	if (!TryGetAmmoData(AmmoType, _outAmmoData))
		return false;

	ChangeTurretAmmo(IsMainTurret, AmmoType, _outAmmoData->ReloadCapacity.Value.MaxValue);
	return true;
}

void UTurretStateGroup::TryEquipTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	// 서버/클라이언트 모두에서 호출 가능
	ServerEquipTurret(IsMainTurret, AmmoType);
}

void UTurretStateGroup::ServerEquipTurret_Implementation(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	// 서버에서만 실행: 장착가능 여부 체크
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return;

	if (!_outTurretStand->CanEquipTurret())
	{
		FString _turretPosition = IsMainTurret ? TEXT("Main") : CommonEnums::GetEnum2FString<E_AMMO_TYPE>(AmmoType);
		UE_LOG(LogTemp, Warning, TEXT("UTurretStateGroup: Turret already equipped at [%s]"), *_turretPosition);
		return;
	}

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
		return;

	// 검증 통과 시 서버/클라이언트 모두에서 로드·스폰·장착 수행
	MulticastEquipTurret(IsMainTurret, AmmoType);
}

void UTurretStateGroup::MulticastEquipTurret_Implementation(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	// 서버/클라이언트 모두에서 실행: 터렛 BP 로드, 스폰, 장착
	TObjectPtr<ATurretStand> _outTurretStand = nullptr;
	if (!TryGetTurretStand(IsMainTurret, AmmoType, _outTurretStand))
		return;

	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
		return;;

	// 터렛 BP 클래스 로드
	FString _fileName = "BP_" + CommonEnums::GetEnum2FString<E_TURRET_TYPE>(_outTurretData->TurretType);
	FString _turretBPPath = ConstantLibrary::Resource.TurretBP.TURRET_BP_FOLDER_PATH + _fileName + "." + _fileName + "_C";
	TSubclassOf<AActor> _turretClass = LoadClass<AActor>(nullptr, *_turretBPPath);
	if (_turretClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load turret BP class: %s"), *_turretBPPath);
		return;
	}

	// 스폰은 서버에서만 (복제로 클라이언트에 전달)
	AActor* _spawnedTurret = nullptr;
	if (GetOwner()->HasAuthority())
	{
		FVector _spawnLocation = _outTurretStand->GetActorLocation() + _outTurretStand->GetActorUpVector() * 1.0f;
		FActorSpawnParameters _spawnParams;
		_spawnParams.Owner = _outTurretStand;
		_spawnedTurret = GetWorld()->SpawnActor<AActor>(_turretClass, _spawnLocation, _outTurretStand->GetActorRotation(), _spawnParams);
		if (_spawnedTurret == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to spawn turret: %s"), *_turretBPPath);
			return;
		}

		// BP 기본값과 무관하게 일반 클라이언트에도 보이도록 복제 강제
		_spawnedTurret->SetOwner(_outTurretStand);
		_spawnedTurret->SetReplicates(true);
		_spawnedTurret->SetReplicateMovement(true);
		_spawnedTurret->ForceNetUpdate();
	}

	// 터렛 장착 (서버만 직접 수행, 클라이언트는 복제로 반영)
	if (_spawnedTurret != nullptr)
	{
		bool _isEquiped = _outTurretStand->TryEquipTurret(_spawnedTurret);
		if (IsMainTurret && _isEquiped && _turretChair != nullptr)
		{
			_turretChair->SetTurretPawn(Cast<APawn>(_spawnedTurret));
			_outTurretStand->SetTurretType(AmmoType);
		}
	}
}

void UTurretStateGroup::LoadTurretDataTable()
{
	// Turret Data
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _dataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));
	if (!_dataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load Turret Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _dataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FTurretInitState* _turrerInfo = _dataTable->FindRow<FTurretInitState>(_rowName, TEXT(""));
		if (_turrerInfo)
		{
			FTurretData _newTurretData;
			// 터렛 정보
			_newTurretData.TurretType = _turrerInfo->TurretType;
			_newTurretData.AmmoType = _turrerInfo->AmmoType;
			_newTurretData.IsMainTurret = _turrerInfo->bIsMainTurret;
			TObjectPtr<UTexture2D> _outTexture = nullptr;
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_TURRET_TYPE>(_newTurretData.TurretType);
			if (AJHSGameState::TryGetTextureFromPath(ConstantLibrary::Resource.Image.TURRET_FOLDER_PATH, _fileName, _outTexture))
			{
				_newTurretData.TurretImage = _outTexture;
			}

			// 가격
			FPurchaseDataFormat _pricePurchaseDataFormat;
			_pricePurchaseDataFormat.Description = _turrerInfo->Description;
			_pricePurchaseDataFormat.MaxLevel = 1;
			_pricePurchaseDataFormat.InitValue = 0;
			_pricePurchaseDataFormat.IncreasePerValue = 100.0f;
			_pricePurchaseDataFormat.InitDollar = _turrerInfo->Price;
			_pricePurchaseDataFormat.IncreasePerDollar = 0.0f;

			_newTurretData.Price = AJHSGameState::ParseFromDataRow(_newTurretData.TurretImage, _pricePurchaseDataFormat);;

			// 탄약
			_newTurretData.Mag = AJHSGameState::ParseFromDataRow(_newTurretData.TurretImage, _turrerInfo->Mag);

			// 공격 속도
			_newTurretData.FireInterval = AJHSGameState::ParseFromDataRow(_newTurretData.TurretImage, _turrerInfo->FireInterval);

			_turretDataMap.Add(GetTurretKey(_newTurretData.IsMainTurret, _newTurretData.AmmoType), _newTurretData);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Turret Data Table loaded successfully. %d rows loaded"), _turretDataMap.Num());

	// Ammo Data
	_ammoDataMap.Empty();

	_dataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.AMMO_INFO_PATH));
	if (!_dataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load Ammo Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.AMMO_INFO_PATH);
		return;
	}

	_rowNames = _dataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FAmmoInitState* _ammoInfo = _dataTable->FindRow<FAmmoInitState>(_rowName, TEXT(""));
		if (_ammoInfo)
		{
			FAmmoData _newAmmoData;
			// 터렛 정보
			_newAmmoData.AmmoType = _ammoInfo->AmmoType;
			TObjectPtr<UTexture2D> _outTexture = nullptr;
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_AMMO_TYPE>(_newAmmoData.AmmoType);
			if (AJHSGameState::TryGetTextureFromPath(ConstantLibrary::Resource.Image.AMMO_FOLDER_PATH, _fileName, _outTexture))
			{
				_newAmmoData.AmmoImage = _outTexture;
			}

			// 가격
			FPurchaseData _price;
			_price.Image = _newAmmoData.AmmoImage;
			_price.Description = FString::Printf(TEXT("%s"), *_ammoInfo->Description);
			_price.Level.MaxValue = -1;
			_price.PurchaseDollar = _ammoInfo->Price;
			_newAmmoData.Price = _price;

			// 재장전 용량
			_newAmmoData.ReloadCapacity = AJHSGameState::ParseFromDataRow(_newAmmoData.AmmoImage, _ammoInfo->ReloadCapacity);

			// 탄약 데미지
			_newAmmoData.AmmoDamage = AJHSGameState::ParseFromDataRow(_newAmmoData.AmmoImage, _ammoInfo->AmmoDamage);

			_ammoDataMap.Add(_newAmmoData.AmmoType, _newAmmoData);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Ammo Data Table loaded successfully. %d rows loaded"), _turretDataMap.Num());
	SyncTurretStateToReplicated();
}

void UTurretStateGroup::SyncTurretStateToReplicated()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		_replicatedTurretDataArray.Empty();
		for (const auto& _pair : _turretDataMap)
			_replicatedTurretDataArray.Add(_pair.Value);
		_replicatedAmmoDataArray.Empty();
		for (const auto& _pair : _ammoDataMap)
			_replicatedAmmoDataArray.Add(_pair.Value);
	}
}

int32 UTurretStateGroup::GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	return ((int32)IsMainTurret + 1) * HUNDRED + (int32)AmmoType;
}

bool UTurretStateGroup::TryGetTurretData(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData)
{
	int32 _turretKey = GetTurretKey(IsMainTurret, AmmoType);
	if (!_turretDataMap.Contains(_turretKey))
	{
		FString _isMainPosition = IsMainTurret ? TEXT("Main") : TEXT("Auto");
		FString _ammoType = CommonEnums::GetEnum2FString<E_AMMO_TYPE>(AmmoType);
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Invalid TurretKey,\nIsMainTurret: [%s], AmmoType: [%s]"), *_isMainPosition, *_ammoType);
		return false;
	}

	OutTurretData = _turretDataMap.Find(_turretKey);
	return OutTurretData != nullptr;
}

bool UTurretStateGroup::TryGetAmmoData(E_AMMO_TYPE AmmoType, FAmmoData*& OutAmmoData)
{
	if (!_ammoDataMap.Contains(AmmoType))
	{
		FString _ammoType = CommonEnums::GetEnum2FString<E_AMMO_TYPE>(AmmoType);
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Invalid AmmoType: [%s]"), *_ammoType);
		return false;
	}

	OutAmmoData = _ammoDataMap.Find(AmmoType);
	return OutAmmoData != nullptr;
}

bool UTurretStateGroup::TryGetTurretStand(bool IsMainTurret, E_AMMO_TYPE AmmoType, TObjectPtr<ATurretStand>& OutTurretStand)
{
	if (IsMainTurret)
		AmmoType = E_AMMO_TYPE::NONE;

	if (!_turretStandMap.Contains(AmmoType))
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Not initialized turret state. TurretPosition: %s"), *CommonEnums::GetEnum2FString<E_AMMO_TYPE>(AmmoType));
		return false;
	}

	OutTurretStand = _turretStandMap.FindRef(AmmoType);
	return OutTurretStand != nullptr;
}

void UTurretStateGroup::ChangeTurretAmmo(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 ChangeValue)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(IsMainTurret, AmmoType, _outTurretData))
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

	ExecuteTurretEvent(IsMainTurret, AmmoType, *_outTurretData);
	SyncTurretStateToReplicated();
}

void UTurretStateGroup::ExecuteTurretEvent(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData TurretData)
{
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	_event->IsMainTurret = IsMainTurret;
	_event->TurretData = TurretData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
}