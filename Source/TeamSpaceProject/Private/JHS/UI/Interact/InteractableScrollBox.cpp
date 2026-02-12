// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Interact/InteractableScrollBox.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Types/SlateConstants.h"
#include "Styling/SlateTypes.h"

void UInteractableScrollBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInteractableScrollBox::NativeConstruct()
{
	Super::NativeConstruct();

	ChangeScrollMode(false);
}

void UInteractableScrollBox::OnHover()
{
	Super::OnHover();
}

void UInteractableScrollBox::OnUnhover()
{
	Super::OnUnhover();

	ChangeScrollMode(false);
}

void UInteractableScrollBox::OnClickEnter()
{
	Super::OnClickEnter();
	
	ChangeScrollMode(!_isScrollMode);
}

void UInteractableScrollBox::OnClickExit()
{
	Super::OnClickExit();
}

bool UInteractableScrollBox::WantsScrollInput() const
{
	return _isScrollMode;
}

void UInteractableScrollBox::ProcessScrollInput(float DeltaY)
{
	if (SCRL_ScrollBox == nullptr || !_isScrollMode)
	{
		return;
	}

	// DeltaY: 마우스 휠 축 값 (양수=휠 업, 음수=휠 다운). 스크롤 오프셋에 적용
	const float _currentOffset = SCRL_ScrollBox->GetScrollOffset();
	const float _maxOffset = SCRL_ScrollBox->GetScrollOffsetOfEnd();
	const float _delta = -DeltaY * GetGlobalScrollAmount() * _scrollSensitivity;
	const float _newOffset = FMath::Clamp(_currentOffset + _delta, 0.0f, _maxOffset);

	SCRL_ScrollBox->SetScrollOffset(_newOffset);
}

void UInteractableScrollBox::AddChild(UWidget* Content)
{
	SCRL_ScrollBox->AddChild(Content);
}

void UInteractableScrollBox::RemoveChild(UWidget* Content)
{
	SCRL_ScrollBox->RemoveChild(Content);
}

void UInteractableScrollBox::ClearChildren()
{
	SCRL_ScrollBox->ClearChildren();
}

void UInteractableScrollBox::ChangeScrollMode(bool IsScrollMode)
{
	_isScrollMode = IsScrollMode;
	ESlateVisibility _visibility = _isScrollMode ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	IMG_OnHover->SetVisibility(_visibility);
}