// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableContainerUI.h"
#include "JHS/UI/UIBase.h"

void AInteractableContainerUI::OnInteractEnter(TObjectPtr<UUIBase> OpenedUI)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact enter"));
}

void AInteractableContainerUI::OnInteractExit(TObjectPtr<UUIBase> ClosedUI)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact exit"));
}