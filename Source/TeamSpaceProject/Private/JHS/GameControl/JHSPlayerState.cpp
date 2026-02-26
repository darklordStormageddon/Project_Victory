// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSPlayerState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "Net/UnrealNetwork.h"

void AJHSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJHSPlayerState, _assignedPlayerId);
}

void AJHSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	OpenCommonInfoUI();
}

bool AJHSPlayerState::TryRegistPlayer(FString Name, E_REGIST_ERROR_TYPE& OutErrorType)
{
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return false;

	return _outGameState->GetPlayerStateGroup()->TryRegistPlayer(this, Name, OutErrorType);
}

void AJHSPlayerState::SetAssignedPlayerId(int32 AssignedPlayerId)
{
	_assignedPlayerId = AssignedPlayerId;
}

void AJHSPlayerState::OpenCommonInfoUI()
{
	// PlayerState가 소유한 PlayerController 가져오기
	APlayerController* _myPlayerController = GetPlayerController();
	if (_myPlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSPlayerState::OpenCommonInfoUI - PlayerController is nullptr"));
		return;
	}

	// 해당 PlayerController의 UIManager 가져오기 (위젯은 로컬 플레이어 컨트롤러에만 할당 가능)
	AJHSPlayerController* _jhsPlayerController = Cast<AJHSPlayerController>(_myPlayerController);
	if (_jhsPlayerController == nullptr || !_jhsPlayerController->IsLocalPlayerController())
	{
		if (_jhsPlayerController == nullptr)
			UE_LOG(LogTemp, Error, TEXT("AJHSPlayerState::OpenCommonInfoUI - PlayerController is not AJHSPlayerController"));
		return;
	}

	UUIManager* _uiManager = _jhsPlayerController->GetUIManager();
	if (_uiManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSPlayerState::OpenCommonInfoUI - UIManager is nullptr"));
		return;
	}

	_uiManager->OpenUI(E_UI_TYPE::UIPanelCommonInfo);
}