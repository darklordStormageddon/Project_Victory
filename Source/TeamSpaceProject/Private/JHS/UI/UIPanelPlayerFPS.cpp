// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIPanelPlayerFPS.h"

void UUIPanelPlayerFPS::InitializeUI()
{
	ChangeInteractable(false);
}

void UUIPanelPlayerFPS::ChangeInteractable(bool IsInteract)
{
	_isInteractable = IsInteract;
	if (_isInteractable)
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Hidden);
	}
}