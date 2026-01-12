// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/InteractableChairExample.h"

void AInteractableChairExample::OnInteractEnter()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact enter"));
}

void AInteractableChairExample::OnInteractExit()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact exit"));
}