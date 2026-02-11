// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Panel/Container/UIPanelContainer.h"
#include "JHS/UI/Panel/Container/PlateContainer.h"
#include "Components/SizeBox.h"

void UUIPanelContainer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SB_PlateContainer == nullptr || _plateContainerClass == nullptr)
		return;

	APlayerController* _playerController = GetOwningPlayer();
	if (_playerController == nullptr)
		return;

	_plateContainer = CreateWidget<UPlateContainer>(_playerController, _plateContainerClass);
	if (_plateContainer != nullptr)
	{
		SB_PlateContainer->AddChild(_plateContainer);
	}
}

void UUIPanelContainer::OnOpen()
{
	Super::OnOpen();

	if (_plateContainer != nullptr)
	{
		_plateContainer->Open();
	}
}
