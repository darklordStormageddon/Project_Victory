// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIInteracter.h"
#include "JHS/UI/UIInteracterable.h"
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

void UUIInteracter::OnInteractable(TObjectPtr<UUIInteracterable> UIInteractable)
{
	_uiInteractable = UIInteractable;
	_uiPanelPlayer->ChangeInteractable(true);
}

void UUIInteracter::OnDisInteractable()
{
	_uiInteractable = nullptr;
	_uiPanelPlayer->ChangeInteractable(false);
	_uiPanelPlayer->Open();
}

void UUIInteracter::InteractInput()
{
	if (_uiInteractable == nullptr)
		return;

	if (_uiInteractable->TryInteract())
	{
		_uiPanelPlayer->Close();
	}
	else
	{
		_uiPanelPlayer->Open();
	}
}