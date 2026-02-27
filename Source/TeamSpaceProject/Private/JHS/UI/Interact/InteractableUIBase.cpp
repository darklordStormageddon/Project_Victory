// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Interact/InteractableUIBase.h"

void UInteractableUIBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetClickable(true);
}

void UInteractableUIBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInteractableUIBase::SetClickable(bool IsClickable)
{
	_isClickable = IsClickable;
	OnChangeClickable(_isClickable);
}

void UInteractableUIBase::Focus()
{
	if (!_isClickable)
		return;

	OnHover();
	Hovered.Broadcast();
}

void UInteractableUIBase::Unfocus()
{
	if (!_isClickable)
		return;

	OnUnhover();
	Unhovered.Broadcast();
}

void UInteractableUIBase::ClickEnter()
{
	if (!_isClickable)
		return;

	OnClickEnter();
	ClickedEnter.Broadcast();
}

void UInteractableUIBase::ClickExit()
{
	if (!_isClickable)
		return;

	OnClickExit();
	ClickedExit.Broadcast();
}