// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Interact/InteractableComponent.h"
#include "Net/UnrealNetwork.h"

AJHSPlayerController::AJHSPlayerController()
{
	_uiManager = CreateDefaultSubobject<UUIManager>(TEXT("UIManager"));
}

void AJHSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AJHSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJHSPlayerController, _assignedPlayerId);
}

void AJHSPlayerController::SetAssignedPlayerId(int32 AssignedPlayerId)
{
	_assignedPlayerId = AssignedPlayerId;
}

void AJHSPlayerController::ClientInteractableTriggerEnter_Implementation(UInteractableComponent* Interactable, E_INTERACT_TYPE InteractType)
{
	if (IsValid(Interactable))
		Interactable->ExecuteTriggerEnterForLocalPlayer(InteractType);
}

void AJHSPlayerController::ClientInteractableTriggerExit_Implementation(UInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
		Interactable->ExecuteTriggerExitForLocalPlayer();
}
