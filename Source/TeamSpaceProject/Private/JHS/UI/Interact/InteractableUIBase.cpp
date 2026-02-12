// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Interact/InteractableUIBase.h"

void UInteractableUIBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInteractableUIBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInteractableUIBase::Focus()
{
	OnHovered();
	OnHover();
	Hovered.Broadcast();
}

void UInteractableUIBase::Unfocus()
{
	OnUnhovered();
	OnUnhover();
	Unhovered.Broadcast();
}

void UInteractableUIBase::ClickEnter()
{
	OnClickedEnter();
	OnClickEnter();
	ClickedEnter.Broadcast();
	Clicked.Broadcast();
}

void UInteractableUIBase::ClickExit()
{
	OnClickedExit();
	OnClickExit();
	ClickedExit.Broadcast();
}