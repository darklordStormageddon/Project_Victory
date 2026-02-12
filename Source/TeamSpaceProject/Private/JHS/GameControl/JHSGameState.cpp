// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "Kismet/GameplayStatics.h"

AJHSGameState::AJHSGameState()
{
	_rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = _rootComponent;

	_spaceShipStateGroup = CreateDefaultSubobject<USpaceShipStateGroup>(TEXT("SpaceShipStateGroup"));

	_playerStateGroup = CreateDefaultSubobject<UPlayerStateGroup>(TEXT("PlayerStateGroup"));

	_collectStateGroup = CreateDefaultSubobject<UCollectStateGroup>(TEXT("CollectStateGroup"));

	_containerStateGroup = CreateDefaultSubobject<UContainerStateGroup>(TEXT("ContainerStateGroup"));

	_turretStateGroup = CreateDefaultSubobject<UTurretStateGroup>(TEXT("TurretStateGroup"));
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

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_outUIManager->OpenUI(E_UI_TYPE::UIPanelCommonInfo);
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
	if (_cachedEventManager == nullptr)
	{
		UEventManager* _outEventManager = nullptr;
		if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
			return nullptr;

		_cachedEventManager = _outEventManager;
	}

	if (_cachedEventManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to get EventManager"));
		return nullptr;
	}

	return _cachedEventManager;
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

	_newPurchaseData.IncreasePerValue = PurchaseDataFormat.IncreasePerValue;
	_newPurchaseData.PurchaseDollar = PurchaseDataFormat.InitDollar;
	_newPurchaseData.IncreasePerDollar = PurchaseDataFormat.IncreasePerDollar;

	return _newPurchaseData;
}