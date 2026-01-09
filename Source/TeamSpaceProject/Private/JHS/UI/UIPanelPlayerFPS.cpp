// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIPanelPlayerFPS.h"

void UUIPanelPlayerFPS::InitializeUI()
{
	ChangeInteractable(E_INTERACT_TYPE::None);
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
	if (InteractType == E_INTERACT_TYPE::None)
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Visible);
	}
}