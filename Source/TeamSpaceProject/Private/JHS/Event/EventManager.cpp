// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Event/EventManager.h"

UEventManager::UEventManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEventManager::BeginPlay()
{
	Super::BeginPlay();
}

void UEventManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clean();
	Super::EndPlay(EndPlayReason);
}

void UEventManager::Clean()
{
	_eventDelegates.Empty();
	_delegateLookup.Empty();
}
