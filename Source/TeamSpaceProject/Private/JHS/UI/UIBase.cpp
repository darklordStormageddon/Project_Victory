// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIBase.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UUIBase::UUIBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, _isInitialized(false)
	, _isActive(false)
{
}

void UUIBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!_isInitialized)
	{
		_isInitialized = true;
		_isActive = false;
	}

	RegisterEvent();
}

void UUIBase::NativeDestruct()
{
	UnregisterEvent();

	Super::NativeDestruct();
}

void UUIBase::Open()
{
	_isActive = true;
	
	if (!IsInViewport())
	{
		AddToViewport();
	}

	SetVisibility(ESlateVisibility::Visible);
	OnOpen();
}

void UUIBase::Close()
{
	_isActive = false;

	if (IsInViewport())
	{
		RemoveFromParent();
	}

	OnClose();
}

void UUIBase::SetAsFirstSibling()
{
	// UserWidget의 ZOrder는 AddToViewport의 ZOrder 파라미터로 설정
	// 이미 뷰포트에 추가된 경우 RemoveFromParent 후 다시 AddToViewport 필요
	if (IsInViewport())
	{
		RemoveFromParent();
		AddToViewport(0);
	}
}

void UUIBase::SetAsLastSibling()
{
	// UserWidget의 ZOrder는 AddToViewport의 ZOrder 파라미터로 설정
	// 이미 뷰포트에 추가된 경우 RemoveFromParent 후 다시 AddToViewport 필요
	if (IsInViewport())
	{
		RemoveFromParent();
		AddToViewport(INT_MAX);
	}
}

void UUIBase::SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar, UTextBlock* TextBlock)
{
	if (ProgressBar == nullptr)
		return;

	float _percent = MaxValue > 0.0f ? (CurrentValue / MaxValue) : 0.0f;
	ProgressBar->SetPercent(_percent);

	if (TextBlock != nullptr)
	{
		FString _text = FString::Printf(TEXT("%d / %d"), (int32)CurrentValue, (int32)MaxValue);
		TextBlock->SetText(FText::FromString(_text));
		
	}
}