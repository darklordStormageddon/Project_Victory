// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/ShopManager.h"
#include "JHS/Event/EventManager.h"

AJHSGameState::AJHSGameState()
{
	_rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = _rootComponent;

	_spaceShipStateGroup = CreateDefaultSubobject<USpaceShipStateGroup>(TEXT("SpaceShipStateGroup"));

	_playerStateGroup = CreateDefaultSubobject<UPlayerStateGroup>(TEXT("PlayerStateGroup"));

	_collectStateGroup = CreateDefaultSubobject<UCollectStateGroup>(TEXT("CollectStateGroup"));

	_containerStateGroup = CreateDefaultSubobject<UContainerStateGroup>(TEXT("ContainerStateGroup"));

	_turretStateGroup = CreateDefaultSubobject<UTurretStateGroup>(TEXT("TurretStateGroup"));

	_shopManager = CreateDefaultSubobject<UShopManager>(TEXT("ShopManager"));

	// EventManager: GameState에 두어 서버·클라이언트 모두에서 접근 가능 (GameMode는 클라이언트에 없음)
	_eventManager = CreateDefaultSubobject<UEventManager>(TEXT("EventManager"));
}

void AJHSGameState::BeginPlay()
{
	Super::BeginPlay();

	InitializeGameState();
}

void AJHSGameState::InitializeGameState()
{
	_spaceShipStateGroup->InitializeSpaceShipState(this);

	_playerStateGroup->InitializePlayerState(this, _playerRadiation);

	_collectStateGroup->InitializeCollectState(this);

	_containerStateGroup->InitializeContainerState(this, _initContainerState);
	
	_turretStateGroup->InitializeTurretState(this);

	_shopManager->InitializeShop(_containerStateGroup);
}

void AJHSGameState::SendCurrentDataEvent()
{
	_spaceShipStateGroup->UpdateSpaceShipState();

	_playerStateGroup->UpdatePlayerState();

	_collectStateGroup->UpdateCollectState();

	_containerStateGroup->UpdateContainerState();

	_turretStateGroup->UpdateTurretState();
}

TObjectPtr<UEventManager> AJHSGameState::GetEventManager()
{
	return _eventManager;
}

bool AJHSGameState::TryGetTextureFromPath(FString FolderPath, FString FileName, TObjectPtr<UTexture2D>& OutTexture)
{
	FString _texturePath = FolderPath + FileName + "." + FileName;
	OutTexture = LoadObject<UTexture2D>(nullptr, *_texturePath);
	if (OutTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameState: Failed to load texture from path: %s"), *_texturePath);
		return false;
	}

	return true;
}

FPurchaseData AJHSGameState::ParseFromDataRow(UTexture2D* Image, FPurchaseDataFormat PurchaseDataFormat)
{
	FPurchaseData _newPurchaseData;
	_newPurchaseData.Image = Image;
	_newPurchaseData.Description = PurchaseDataFormat.Description;

	_newPurchaseData.Level.MaxValue = PurchaseDataFormat.MaxLevel;
	_newPurchaseData.Level.CurrentValue = 1;

	_newPurchaseData.Value.MaxValue = PurchaseDataFormat.InitValue;
	_newPurchaseData.Value.CurrentValue = _newPurchaseData.Value.MaxValue;
	_newPurchaseData.InitValue = _newPurchaseData.Value.MaxValue;

	_newPurchaseData.IncreasePerValue = PurchaseDataFormat.IncreasePerValue;
	_newPurchaseData.PurchaseDollar = PurchaseDataFormat.InitDollar;
	_newPurchaseData.IncreasePerDollar = PurchaseDataFormat.IncreasePerDollar;
	_newPurchaseData.InitDollar = _newPurchaseData.PurchaseDollar;

	return _newPurchaseData;
}