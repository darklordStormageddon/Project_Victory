// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Panel/Container/UIPanelContainer.h"
#include "JHS/UI/Panel/Container/PlateContainer.h"
#include "Components/SizeBox.h"

void UUIPanelContainer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!SB_PlateContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelContainer: SB_PlateContainer is nullptr"));
		return;
	}
	if (!_plateContainerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelContainer: _plateContainerClass is nullptr"));
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelContainer: Owning player is nullptr"));
		return;
	}

	_plateContainer = CreateWidget<UPlateContainer>(PlayerController, _plateContainerClass);
	if (!_plateContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelContainer: Failed to create PlateContainer"));
		return;
	}

	SB_PlateContainer->AddChild(_plateContainer);
}

void UUIPanelContainer::OnOpen()
{
	Super::OnOpen();
}
