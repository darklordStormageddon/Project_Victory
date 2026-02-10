// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableStageController.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"

void AInteractableStageController::OnInteractEnter(AActor* Caller, TObjectPtr<UUIBase> OpenedUI)
{
	InteractController(Caller);
}

void AInteractableStageController::OnInteractExit(AActor* Caller, TObjectPtr<UUIBase> ClosedUI)
{
}

void AInteractableStageController::InteractController(AActor* Caller)
{
	AJHSGameMode* _outGameMode = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
		return;

	if (_isStartStage)
	{
		_outGameMode->StartNextStage(Caller);
	}
	else
	{
		_outGameMode->EndStage(Caller);
	}
}