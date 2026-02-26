// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/UI/Interact/InteractableUIBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/WidgetComponent.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UUIBase::UUIBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, _isInitialized(false)
	, _isActive(false)
{
}

void UUIBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
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

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_outGameState->SendCurrentDataEvent();

	SetVisibility(ESlateVisibility::Visible);

	OnOpen();

	UpdateHostCollision();
}

void UUIBase::SetHostWidgetComponent(UWidgetComponent* InComponent)
{
	_hostWidgetComponent = InComponent;
	if (UWidgetComponent* _wc = _hostWidgetComponent.Get())
	{
		_wc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UUIBase::UpdateHostCollision()
{
	UWidgetComponent* _wc = _hostWidgetComponent.Get();
	if (_wc == nullptr)
	{
		return;
	}

	TArray<UWidget*> _allWidgets;
	UWidget* _treeRoot = (WidgetTree && WidgetTree->RootWidget) ? WidgetTree->RootWidget : this;
	_allWidgets.Add(_treeRoot);
	TSet<UWidget*> _seen;
	_seen.Add(_treeRoot);

	for (int32 _i = 0; _i < _allWidgets.Num(); ++_i)
	{
		UWidget* _w = _allWidgets[_i];
		if (UPanelWidget* _panel = Cast<UPanelWidget>(_w))
		{
			const int32 _childCount = _panel->GetChildrenCount();
			for (int32 _c = 0; _c < _childCount; ++_c)
			{
				if (UWidget* _child = _panel->GetChildAt(_c))
				{
					_allWidgets.Add(_child);
					_seen.Add(_child);
				}
			}
		}
		else if (UUserWidget* _userW = Cast<UUserWidget>(_w))
		{
			if (_userW->WidgetTree && _userW->WidgetTree->RootWidget)
			{
				UWidget* _innerRoot = _userW->WidgetTree->RootWidget;
				if (!_seen.Contains(_innerRoot))
				{
					_allWidgets.Add(_innerRoot);
					_seen.Add(_innerRoot);
				}
			}
		}
	}

	bool _hasInteractable = false;
	for (UWidget* _w : _allWidgets)
	{
		if (Cast<UInteractableUIBase>(_w) != nullptr)
		{
			_hasInteractable = true;
			break;
		}
	}

	if (_hasInteractable)
	{
		_wc->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		_wc->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
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

void UUIBase::SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar, UTextBlock* TextBlock, bool IsOnlyCurrentText)
{
	if (ProgressBar == nullptr)
		return;

	float _percent = MaxValue > 0.0f ? (CurrentValue / MaxValue) : 0.0f;
	ProgressBar->SetPercent(_percent);

	if (TextBlock != nullptr)
	{
		FString _text;
		if (IsOnlyCurrentText)
		{
			_text = FString::Printf(TEXT("%d"), (int32)CurrentValue);
		}
		else
		{
			_text = FString::Printf(TEXT("%d / %d"), (int32)CurrentValue, (int32)MaxValue);
		}
		TextBlock->SetText(FText::FromString(_text));
	}
}

void UUIBase::SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar)
{
	SetProgressBarUI(CurrentValue, MaxValue, ProgressBar, nullptr, false);
}

TObjectPtr<UEventManager> UUIBase::GetEventManager()
{
	if (_cachedEventManager == nullptr)
	{
		UEventManager* _outEventManager = nullptr;
		if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
			return nullptr;

		_cachedEventManager = _outEventManager;
	}

	return _cachedEventManager;
}