// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
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
	{
		Interactable->ExecuteTriggerEnterForLocalPlayer(InteractType);
	}
}

void AJHSPlayerController::ClientInteractableTriggerExit_Implementation(UInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
	{
		Interactable->ExecuteTriggerExitForLocalPlayer();
	}
}

void AJHSPlayerController::ServerRequestToggleWorldUI_Implementation(UInteractableComponent* Target)
{
	if (IsValid(Target))
	{
		Target->AuthorityToggleWorldUI();
	}
}

void AJHSPlayerController::ServerRequestPurchaseSpaceShip_Implementation(E_SPACE_SHIP_DATA_TYPE DataType)
{
	AJHSGameState* _gameState = GetWorld()->GetGameState<AJHSGameState>();
	if (_gameState != nullptr && _gameState->GetSpaceShipStateGroup() != nullptr)
	{
		_gameState->GetSpaceShipStateGroup()->ExecutePurchaseSpaceShipData(DataType);
	}
}

void AJHSPlayerController::ServerRequestPurchaseCollectTool_Implementation(E_COLLECT_TOOL_TYPE ToolType, bool IsDurability)
{
	AJHSGameState* _gameState = GetWorld()->GetGameState<AJHSGameState>();
	if (_gameState != nullptr && _gameState->GetCollectStateGroup() != nullptr)
	{
		_gameState->GetCollectStateGroup()->ExecutePurchaseCollectTool(ToolType, IsDurability);
	}
}

void AJHSPlayerController::ServerRequestPurchaseTurret_Implementation(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	AJHSGameState* _gameState = GetWorld()->GetGameState<AJHSGameState>();
	if (_gameState != nullptr && _gameState->GetTurretStateGroup() != nullptr)
	{
		_gameState->GetTurretStateGroup()->ExecutePurchaseTurret(IsMainTurret, AmmoType, FieldIndex);
	}
}

void AJHSPlayerController::ServerRequestPurchaseAmmo_Implementation(E_AMMO_TYPE AmmoType, int32 FieldIndex)
{
	AJHSGameState* _gameState = GetWorld()->GetGameState<AJHSGameState>();
	if (_gameState != nullptr && _gameState->GetTurretStateGroup() != nullptr)
	{
		_gameState->GetTurretStateGroup()->ExecutePurchaseAmmo(AmmoType, FieldIndex);
	}
}

void AJHSPlayerController::ServerRequestSaleAllElement_Implementation()
{
	AJHSGameState* _gameState = GetWorld()->GetGameState<AJHSGameState>();
	if (_gameState != nullptr && _gameState->GetContainerStateGroup() != nullptr)
	{
		_gameState->GetContainerStateGroup()->ServerSaleAllElement();
	}
}
