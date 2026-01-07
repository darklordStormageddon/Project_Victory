// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

TObjectPtr<UEventManager> AJHSGameState::GetEventManager()
{
	if (_cachedEventManager == nullptr)
	{
		AJHSGameMode* _outGameMode = nullptr;
		if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
			return nullptr;

		_cachedEventManager = _outGameMode->GetEventManager();
	}

	return _cachedEventManager;
}

void AJHSGameState::ChangeSpaceShipData(FSpaceShipData SpaceShipData)
{
	_spaceShipData = SpaceShipData;

	// 이벤트 발생 (Unity의 ExecuteEvent(new UEventOnChangeSpaceShipData(_spaceShipData))와 동일)
	UEventManager* _eventManager = GetEventManager();
	if (_eventManager != nullptr)
	{
		UEventOnChangeSpaceShipData* _event = NewObject<UEventOnChangeSpaceShipData>(this);
		_event->SpaceShipData = _spaceShipData;
		_eventManager->ExecuteEvent<UEventOnChangeSpaceShipData>(_event);
	}
}