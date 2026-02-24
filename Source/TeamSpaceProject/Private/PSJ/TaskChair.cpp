// Fill out your copyright notice in the Description page of Project Settings.


#include "PSJ/TaskChair.h"
#include "JHS/UI/UIBase.h"
#include "PSJ/TaskPawnBase.h"
#include "Net/UnrealNetwork.h"
#include "PSJ_Character.h" 

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

	if (TargetTaskPawn == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ATaskChair: TargetTaskPawn is nullptr"));
		return;
	}

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);

	if (!MyChar) return;

	MyChar->Server_RequestBoarding(TargetTaskPawn);
}

void ATaskChair::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
	Super::OnInteractExit(CallerPlayerId, ClosedUI);
}

void ATaskChair::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}