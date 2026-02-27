// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Interact/InteractableButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInteractableButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Button == nullptr)
		return;

	BTN_Button->OnHovered.AddDynamic(this, &UInteractableButton::Focus);
	BTN_Button->OnUnhovered.AddDynamic(this, &UInteractableButton::Unfocus);
	BTN_Button->OnClicked.AddDynamic(this, &UInteractableButton::OnButtonClicked);
}

void UInteractableButton::NativeConstruct()
{
	Super::NativeConstruct();

	IMG_OnHover->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractableButton::OnChangeClickable(bool IsClickable)
{
	ESlateVisibility _visibility = IsClickable ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
	IMG_BlockClick->SetVisibility(_visibility);
}

void UInteractableButton::OnHover()
{
	Super::OnHover();

	IMG_OnHover->SetVisibility(ESlateVisibility::Visible);
}

void UInteractableButton::OnUnhover()
{
	Super::OnUnhover();

	IMG_OnHover->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractableButton::OnClickEnter()
{
	Super::OnClickEnter();
}

void UInteractableButton::OnClickExit()
{
	Super::OnClickExit();
}

void UInteractableButton::InitializeButton(FString Label)
{
	TXT_Label->SetText(FText::FromString(Label));
}

void UInteractableButton::OnButtonClicked()
{
	ClickEnter();
	ClickExit();
}