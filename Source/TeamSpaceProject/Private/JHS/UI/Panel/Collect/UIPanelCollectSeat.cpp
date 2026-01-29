// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Collect/UIPanelCollectSeat.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/UI/Panel/Collect/CollectToolDurability.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"

void UUIPanelCollectSeat::NativeOnInitialized()
{
	ClearDynamicWidgets();

	// 템플릿 위젯을 HorizontalBox에서 분리하여 레이아웃에 영향 없도록 함
	if (WBP_CollectToolDurability && WBP_CollectToolDurability->GetParent())
	{
		WBP_CollectToolDurability->RemoveFromParent();
	}

	// Spacer들의 크기를 Fill로 설정
	if (Spacer_Left)
	{
		UHorizontalBoxSlot* _leftSlot = Cast<UHorizontalBoxSlot>(Spacer_Left->Slot);
		if (_leftSlot)
		{
			_leftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}

	if (Spacer_Right)
	{
		UHorizontalBoxSlot* _rightSlot = Cast<UHorizontalBoxSlot>(Spacer_Right->Slot);
		if (_rightSlot)
		{
			_rightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}
}

void UUIPanelCollectSeat::OnOpen()
{
}

void UUIPanelCollectSeat::OnClose()
{

}

void UUIPanelCollectSeat::RegisterEvent()
{
	_eventHandleOnChangeDurability = GetEventManager()->AddListener<UEventOnChangeToolDurability>(
        [this](UEventOnChangeToolDurability* Event)
        {
			OnChangeDurability(Event);
        }
    );

	_eventHandleOnChangeTool = GetEventManager()->AddListener<UEventOnChangeTool>(
		[this](UEventOnChangeTool* Event)
		{
			OnChangeTool(Event);
		}
	);
}

void UUIPanelCollectSeat::UnregisterEvent()
{
	if (_eventHandleOnChangeDurability.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeToolDurability>(_eventHandleOnChangeDurability);
		_eventHandleOnChangeDurability.Reset();
    }

	if (_eventHandleOnChangeTool.IsValid())
	{
		GetEventManager()->DelListener<UEventOnChangeTool>(_eventHandleOnChangeTool);
		_eventHandleOnChangeTool.Reset();
	}
}

void UUIPanelCollectSeat::OnChangeDurability(UEventOnChangeToolDurability* Event)
{
	if (Event == nullptr)
		return;

	FCollectToolData _collectToolData = Event->CollectToolData;
	TObjectPtr<UCollectToolDurability> _collectToolItem = GetCollectToolItem(_collectToolData.CollectToolType);

	float _progress = FMath::Clamp(_collectToolData.Durability.CurrentValue / _collectToolData.Durability.MaxValue, 0.0f, 1.0f);
	bool _isVaccumTool = _collectToolData.CollectToolType == E_COLLECT_TOOL_TYPE::Vacuum;
	if (_isVaccumTool)
	{
		_progress = 0.0f;
	}
	_collectToolItem->SetDurabilityProgress(_collectToolData.CollectToolImage, _progress, _collectToolData.CollectToolType == E_COLLECT_TOOL_TYPE::Vacuum);
}

void UUIPanelCollectSeat::OnChangeTool(UEventOnChangeTool* Event)
{
	if (Event == nullptr)
		return;

	E_COLLECT_TOOL_TYPE _prevToolType = Event->PrevToolType;
	E_COLLECT_TOOL_TYPE _nextToolType = Event->NextToolType;;
	TObjectPtr<UCollectToolDurability> _collectToolItem = nullptr;
	if (_prevToolType != E_COLLECT_TOOL_TYPE::NONE)
	{
		_collectToolItem = GetCollectToolItem(_prevToolType);
		_collectToolItem->SelectTool(false);
	}

	if (_nextToolType == E_COLLECT_TOOL_TYPE::NONE)
		return;

	_collectToolItem = GetCollectToolItem(_nextToolType);
	_collectToolItem->SelectTool(true);
}

void UUIPanelCollectSeat::ClearDynamicWidgets()
{
	if (!HorizontalBox || !Spacer_Left || !Spacer_Right)
		return;

	// Spacer_Left와 Spacer_Right 사이의 위젯들만 제거
	int32 _spacerLeftIndex = HorizontalBox->GetChildIndex(Spacer_Left);
	int32 _spacerRightIndex = HorizontalBox->GetChildIndex(Spacer_Right);

	if (_spacerLeftIndex == -1 || _spacerRightIndex == -1)
		return;

	// Spacer_Left 다음부터 Spacer_Right 이전까지 제거 (역순으로)
	for (int32 _i = _spacerRightIndex - 1; _i > _spacerLeftIndex; --_i)
	{
		HorizontalBox->RemoveChildAt(_i);
	}

	_toolDurabilityItemMap.Empty();
}

TObjectPtr<UCollectToolDurability> UUIPanelCollectSeat::GetCollectToolItem(E_COLLECT_TOOL_TYPE CollectToolType)
{
	if (_toolDurabilityItemMap.Contains(CollectToolType))
		return _toolDurabilityItemMap.FindRef(CollectToolType);

	// WBP_CollectToolDurability의 위젯 클래스 가져오기 (블루프린트 클래스일 수 있음)
	UClass* _widgetClass = WBP_CollectToolDurability->GetClass();
	if (!_widgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to get widget class from WBP_CollectToolDurability"));
		return nullptr;
	}

	// PlayerController 가져오기 (CreateWidget에 필요)
	APlayerController* _playerController = GetOwningPlayer();
	if (!_playerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to get PlayerController"));
		return nullptr;
	}

	UCollectToolDurability* _toolWidget = CreateWidget<UCollectToolDurability>(_playerController, _widgetClass);
	if (!_toolWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to create widget"));
		return nullptr;
	}
	
	_toolWidget->SetVisibility(ESlateVisibility::Visible);

	// Spacer_Right를 일시적으로 제거 (Tool들을 추가한 후 마지막에 다시 추가하기 위함)
	if (Spacer_Right && Spacer_Right->GetParent())
	{
		HorizontalBox->RemoveChild(Spacer_Right);
	}

	// 마지막에 인텍스에 추가
	int32 _spacerLeftIndex = HorizontalBox->GetChildIndex(Spacer_Left);
	UPanelSlot* _panelSlot = HorizontalBox->AddChildToHorizontalBox(_toolWidget);
	UHorizontalBoxSlot* _slot = Cast<UHorizontalBoxSlot>(_panelSlot);
	if (_slot)
	{
		// 슬롯 크기를 Auto로 설정
		_slot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		_slot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
		_slot->SetHorizontalAlignment(HAlign_Fill);
		_slot->SetVerticalAlignment(VAlign_Fill);
	}

	_toolDurabilityItemMap.Add(CollectToolType, _toolWidget);

	// 모든 Tool 위젯 추가 후, Spacer_Right를 마지막에 다시 추가
	if (Spacer_Right)
	{
		UHorizontalBoxSlot* _rightSlot = HorizontalBox->AddChildToHorizontalBox(Spacer_Right);
		if (_rightSlot)
		{
			_rightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		int32 _finalRightIndex = HorizontalBox->GetChildIndex(Spacer_Right);
	}

	return _toolWidget;
}