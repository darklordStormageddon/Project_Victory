// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableChairExample.h"

void AInteractableChairExample::OnInteractEnter(TObjectPtr<UUIBase> OpenedUI)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact enter"));
}

void AInteractableChairExample::OnInteractExit(TObjectPtr<UUIBase> ClosedUI)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact exit"));
}