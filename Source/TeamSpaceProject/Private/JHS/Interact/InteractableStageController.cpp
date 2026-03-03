// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableStageController.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "PSJ/TaskPawnBase.h"
#include "Kismet/GameplayStatics.h"


void AInteractableStageController::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{
	InteractController(CallerPlayerId);
}

void AInteractableStageController::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
}

void AInteractableStageController::InteractController(int32 CallerPlayerId)
{
	// 플레이어	
	AJHSPlayerController* _outCallerController = nullptr;
	if (!UStaticFunctionLibrary::TryGetPlayerController(_outCallerController))
		return;

	AJHSGameMode* _outGameMode = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameMode(_outCallerController->GetPawn(), _outGameMode))
		return;

	AJHSPlayerController* _callerController = nullptr;
	if (!UStaticFunctionLibrary::TryGetPlayerController(_callerController))
		return;

	if (_isStartStage)
	{
		_outGameMode->StartNextStage(_callerController);
	}
	else
	{
		_outGameMode->EndStage(_callerController);
	}
}