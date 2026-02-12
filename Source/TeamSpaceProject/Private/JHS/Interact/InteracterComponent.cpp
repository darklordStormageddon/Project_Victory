// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteracterComponent.h"
#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Event/EventManager.h"

// Sets default values for this component's properties
UInteracterComponent::UInteracterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteracterComponent::BeginPlay()
{
	Super::BeginPlay();

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_uiManager = _outUIManager;
	_uiManager->OpenUI(_playerUI);
}

void UInteracterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteracterComponent::OnInteractable(TObjectPtr<UInteractableComponent> Interactable, E_INTERACT_TYPE InteractType)
{
	_interactable = Interactable;
	if (_interactable != nullptr)
	{
		ExecuteEventOnChangeInteractType(InteractType);
	}
}

void UInteracterComponent::OnDisInteractable()
{
	_interactable = nullptr;
	_uiManager->OpenUI(_playerUI);
	ExecuteEventOnChangeInteractType(E_INTERACT_TYPE::Idle);
}

bool UInteracterComponent::TryInteractInput(bool& OutIsInterupt, bool& OutIsInteractEnter)
{
	if (_interactable == nullptr)
		return false;

	if (!_interactable->TryInteract(GetOwner(), OutIsInterupt, OutIsInteractEnter))
		return false;

	if (OutIsInteractEnter)
	{
		_uiManager->CloseUI(_playerUI);
	}
	else
	{
		_uiManager->OpenUI(_playerUI);
	}

	return true;
}

void UInteracterComponent::ExecuteEventOnChangeInteractType(E_INTERACT_TYPE InteractType)
{
	UEventOnChangeInteractType* _event = NewObject<UEventOnChangeInteractType>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteracterComponent: Failed to create UEventOnChangeInteractType"));
		return;
	}

	_event->InteractType = InteractType;
	UEventManager::ExecuteEvent<UEventOnChangeInteractType>(_event);
}