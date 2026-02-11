// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/GameControl/StageChangeExample.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

AStageChangeExample::AStageChangeExample()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStageChangeExample::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	UEventManager* _eventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_eventManager) || _eventManager == nullptr)
		return;

	_eventHandleOnStartStage = _eventManager->AddListener<UEventOnStartStage>(
		[this](UEventOnStartStage* Event)
		{
			OnStartStage(Event);
		}
	);

	_eventHandleOnEndStage = _eventManager->AddListener<UEventOnEndStage>(
		[this](UEventOnEndStage* Event)
		{
			OnEndStage(Event);
		}
	);
}

void AStageChangeExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HasAuthority())
		return;

	UEventManager* _eventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_eventManager) || _eventManager == nullptr)
		return;

	if (_eventHandleOnStartStage.IsValid())
	{
		_eventManager->DelListener<UEventOnStartStage>(_eventHandleOnStartStage);
		_eventHandleOnStartStage.Reset();
	}

	if (_eventHandleOnEndStage.IsValid())
	{
		_eventManager->DelListener<UEventOnEndStage>(_eventHandleOnEndStage);
		_eventHandleOnEndStage.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AStageChangeExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStageChangeExample::OnStartStage_Implementation(UEventOnStartStage* Event)
{
	if (Event == nullptr)
		return;

	// TODO: 스테이지 시작 처리
	int32 _stage = Event->Stage;
	UE_LOG(LogTemp, Warning, TEXT("OnStartStage: %d"), _stage);
}

void AStageChangeExample::OnEndStage_Implementation(UEventOnEndStage* Event)
{
	if (Event == nullptr)
		return;

	// TODO: 스테이지 종료 처리
	UE_LOG(LogTemp, Warning, TEXT("OnEndStage"));
}

