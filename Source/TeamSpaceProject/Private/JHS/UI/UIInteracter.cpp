// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIInteracter.h"
#include "JHS/Interact/InteractableBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIPanelPlayerFPS.h"

// Sets default values for this component's properties
UUIInteracter::UUIInteracter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UUIInteracter::BeginPlay()
{
	Super::BeginPlay();

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_uiPanelPlayer = Cast<UUIPanelPlayerFPS>(_outUIManager->OpenUI(_playerUI));
	if (_uiPanelPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIInteracter: Failed to cast UI to UUIPanelPlayerFPS"));
		return;
	}

	_uiPanelPlayer->InitializeUI();
}

void UUIInteracter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUIInteracter::OnInteractable(TObjectPtr<UInteractableBase> Interactable, bool IsInterrupt)
{
	_interactable = Interactable;
	if (_interactable != nullptr)
	{
		E_INTERACT_TYPE _interactType = IsInterrupt ? E_INTERACT_TYPE::DumpThrow : _interactable->GetInteractType();
		_uiPanelPlayer->ChangeInteractable(_interactType);
	}
}

void UUIInteracter::OnDisInteractable()
{
	_interactable = nullptr;
	_uiPanelPlayer->ChangeInteractable(E_INTERACT_TYPE::None);
	_uiPanelPlayer->Open();
}

bool UUIInteracter::TryInteractInput(bool& OutIsInterupt, bool& OutIsInteractEnter)
{
	if (_interactable == nullptr)
		return false;

	if (!_interactable->TryInteract(OutIsInterupt, OutIsInteractEnter))
		return false;

	if (OutIsInterupt)
		return true;

	if (OutIsInteractEnter)
	{
		_uiPanelPlayer->Close();
	}
	else
	{
		_uiPanelPlayer->Open();
	}

	return true;
}