// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Interact/InteractableButton.h"

#include "Components/Button.h"

void UInteractableButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Button == nullptr)
	{
		return;
	}

	BTN_Button->OnHovered.AddDynamic(this, &UInteractableButton::HandleHovered);
	BTN_Button->OnUnhovered.AddDynamic(this, &UInteractableButton::HandleUnhovered);
	BTN_Button->OnClicked.AddDynamic(this, &UInteractableButton::HandleClicked);
}

void UInteractableButton::HandleHovered()
{
	Focus();
}

void UInteractableButton::HandleUnhovered()
{
	Unfocus();
}

void UInteractableButton::HandleClicked()
{
	Click();
}

void UInteractableButton::Focus()
{
	OnHovered();
	Hovered.Broadcast();
}

void UInteractableButton::Unfocus()
{
	OnUnhovered();
	Unhovered.Broadcast();
}

void UInteractableButton::Click()
{
	OnClicked();
	Clicked.Broadcast();
}

void UInteractableButton::OnHovered_Implementation()
{
	// 블루프린트에서 포커스 연출 구현

	UE_LOG(LogTemp, Warning, TEXT("OnHover"));
}

void UInteractableButton::OnUnhovered_Implementation()
{
	// 블루프린트에서 포커스 해제 연출 구현

	UE_LOG(LogTemp, Warning, TEXT("OnUnhover"));
}

void UInteractableButton::OnClicked_Implementation()
{
	// 블루프린트에서 클릭 연출/로직 구현

	UE_LOG(LogTemp, Warning, TEXT("OnClick"));
}

