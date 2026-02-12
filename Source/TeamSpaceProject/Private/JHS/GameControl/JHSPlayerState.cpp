// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSPlayerState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"

bool AJHSPlayerState::TryRegistPlayer(FString Name, E_REGIST_ERROR_TYPE& OutErrorType)
{
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return false;

	return _outGameState->GetPlayerStateGroup()->TryRegistPlayer(this, Name, OutErrorType);
}