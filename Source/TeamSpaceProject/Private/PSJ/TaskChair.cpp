// Fill out your copyright notice in the Description page of Project Settings.


#include "PSJ/TaskChair.h"
#include "JHS/UI/UIBase.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

void ATaskChair::BeginPlay()
{
	Super::BeginPlay();
}

void ATaskChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATaskChair, TargetTaskPawn);
}

void ATaskChair::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{
	Super::OnInteractEnter(CallerPlayerId, OpenedUI);
}

void ATaskChair::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
	Super::OnInteractExit(CallerPlayerId, ClosedUI);
}

void ATaskChair::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}