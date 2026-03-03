// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/JHSPlayerState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPlayerStateGroup::UPlayerStateGroup()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPlayerStateGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPlayerStateGroup, _replicatedPlayerStateArray);
}

void UPlayerStateGroup::OnRep_PlayerStateArray()
{
	_playerStateMap.Empty();
	for (const FPlayerStateData& _data : _replicatedPlayerStateArray)
		_playerStateMap.Add(_data.AssignedPlayerId, _data);
}

void UPlayerStateGroup::SyncPlayerStateToReplicated()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		_replicatedPlayerStateArray.Empty();
		for (const auto& _pair : _playerStateMap)
			_replicatedPlayerStateArray.Add(_pair.Value);
	}
}

// Called when the game starts
void UPlayerStateGroup::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UPlayerStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerStateGroup::InitializePlayerState(TObjectPtr<AJHSGameState> GameState)
{
	_gameState = GameState;
}

void UPlayerStateGroup::UpdatePlayerState()
{
	for (auto& _playerState : _playerStateMap)
	{
		IncreasePlayerRadiation(_playerState.Key, 0.0f);
	}
}

void UPlayerStateGroup::UpdatePlayerRadiation()
{
	TObjectPtr<USpaceShipStateGroup> _spaceShipStateGroup = _gameState->GetSpaceShipStateGroup();
	if (_spaceShipStateGroup == nullptr)
		return;

	FPurchaseData* _outRadiationData = nullptr;
	if (!_spaceShipStateGroup->TryGetRadiationData(_outRadiationData))
		return;

	for (auto& _playerStateData : _playerStateMap)
	{
		_playerStateData.Value.Radiation.MaxValue = _outRadiationData->Value.MaxValue;
		_playerStateData.Value.Radiation.CurrentValue = 0.0f;
	}
}

bool UPlayerStateGroup::TryRegistPlayer(TObjectPtr<AJHSPlayerState> PlayerState, FString PlayerName, E_REGIST_ERROR_TYPE& ErrorType)
{
	ErrorType = E_REGIST_ERROR_TYPE::NONE;

	if (PlayerState == nullptr)
	{
		ErrorType = E_REGIST_ERROR_TYPE::NullPlayerState;
		return false;
	}

	int32 _playerID = PlayerState->GetPlayerId();
	if (_playerStateMap.Contains(_playerID))
	{
		ErrorType = E_REGIST_ERROR_TYPE::DuplicatedUID;
		return false;
	}

	for (auto& _playerStateData : _playerStateMap)
	{
		if (_playerStateData.Value.PlayerName.Equals(PlayerName))
		{
			ErrorType = E_REGIST_ERROR_TYPE::DuplicatedName;
			return false;
		}
	}

	const int32 _assignedId = _playerStateMap.Num();
	FPlayerStateData _newPlayerData;
	_newPlayerData.AssignedPlayerId = _assignedId;
	_newPlayerData.PlayerName = PlayerName;
	_newPlayerData.PlayerIndex = _assignedId;

	AJHSPlayerState* _jhsPS = Cast<AJHSPlayerState>(PlayerState);
	if (_jhsPS != nullptr)
		_jhsPS->SetAssignedPlayerId(_assignedId);

	_playerStateMap.Add(_newPlayerData.AssignedPlayerId, _newPlayerData);
	SyncPlayerStateToReplicated();
	return true;
}

void UPlayerStateGroup::IncreasePlayerRadiation(int32 CallerAssignedPlayerId, float IncreaseValue)
{
	FPlayerStateData* _playerState = _playerStateMap.Find(CallerAssignedPlayerId);
	if (_playerState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlayerStateGroup::IncreasePlayerRadiation - PlayerIdx=%d not found in _playerStateMap, skip"), CallerAssignedPlayerId);
		return;
	}
	
	FMaxCurrentData* _radiationData = &_playerState->Radiation;
	_radiationData->CurrentValue += IncreaseValue;
	bool _isFullRadiation = false;
	if (_radiationData->CurrentValue > _radiationData->MaxValue)
	{
		_radiationData->CurrentValue = _radiationData->MaxValue;
		_isFullRadiation = true;
	}

	UEventOnChangePlayerRadiation* _eventOnRadiation = NewObject<UEventOnChangePlayerRadiation>(this);
	if (_eventOnRadiation == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UPlayerStateGroup: Failed to create UEventOnChangePlayerRadiation"));
		return;
	}

	_eventOnRadiation->CallerAssignedPlayerId = CallerAssignedPlayerId;
	_eventOnRadiation->RadiationData = *_radiationData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangePlayerRadiation>(_eventOnRadiation);

	// Full radiation
	if (_isFullRadiation)
	{
		UEventOnPlayerDied* _eventDied = NewObject<UEventOnPlayerDied>(this);
		if (_eventDied == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UPlayerStateGroup: Failed to create UEventOnPlayerDied"));
			return;
		}

		_eventDied->CallerAssignedPlayerId = CallerAssignedPlayerId;
		_gameState->GetEventManager()->ExecuteEvent<UEventOnPlayerDied>(_eventDied);
	}
	SyncPlayerStateToReplicated();
}

bool UPlayerStateGroup::TryGetPlayerStateData(int32 PlayerUID, FPlayerStateData*& OutPlayerStateData)
{
	if (!_playerStateMap.Contains(PlayerUID))
	{
		UE_LOG(LogTemp, Error, TEXT("UPlayerStateGroup: Invalid PlayerUID: [%d]"), PlayerUID);
		return false;
	}

	OutPlayerStateData = _playerStateMap.Find(PlayerUID);
	return OutPlayerStateData != nullptr;
}