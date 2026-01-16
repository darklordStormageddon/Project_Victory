// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableContainerUI.h"

void AInteractableContainerUI::OnInteractEnter()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact enter"));
}

void AInteractableContainerUI::OnInteractExit()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact exit"));
}