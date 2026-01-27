// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Collect/UIPanelCollectSeat.h"
#include "JHS/UI/Panel/Collect/CollectToolDurability.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"

void UUIPanelCollectSeat::OnOpen()
{
	InitializeCollectSeat(_toolCount);

	// 모든 Tool 위젯의 진행도를 랜덤으로 설정
	for (TObjectPtr<UCollectToolDurability> _toolWidget : _toolDurabilityWidgets)
	{
		if (_toolWidget)
		{
			float _randomProgress = FMath::RandRange(0.0f, 1.0f);
			_toolWidget->SetDurabilityProgress(_randomProgress);
			UE_LOG(LogTemp, Warning, TEXT("UUIPanelCollectSeat: random %.2f for tool widget"), _randomProgress);
		}
	}
}

void UUIPanelCollectSeat::OnClose()
{

}

void UUIPanelCollectSeat::RegisterEvent()
{

}

void UUIPanelCollectSeat::UnregisterEvent()
{

}

void UUIPanelCollectSeat::InitializeCollectSeat(int32 ToolCount)
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

	// WBP_CollectToolDurability의 위젯 클래스 가져오기 (블루프린트 클래스일 수 있음)
	UClass* _widgetClass = WBP_CollectToolDurability->GetClass();
	if (!_widgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to get widget class from WBP_CollectToolDurability"));
		return;
	}

	// PlayerController 가져오기 (CreateWidget에 필요)
	APlayerController* _playerController = GetOwningPlayer();
	if (!_playerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to get PlayerController"));
		return;
	}

	// Spacer_Left 인덱스 찾기
	int32 _spacerLeftIndex = HorizontalBox->GetChildIndex(Spacer_Left);
	
	if (_spacerLeftIndex == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Spacer_Left not found"));
		return;
	}

	// Spacer_Right를 일시적으로 제거 (Tool들을 추가한 후 마지막에 다시 추가하기 위함)
	if (Spacer_Right && Spacer_Right->GetParent())
	{
		HorizontalBox->RemoveChild(Spacer_Right);
	}

	// Spacer_Left 다음 위치부터 위젯 추가
	for (int32 _i = 0; _i < ToolCount; ++_i)
	{
		UCollectToolDurability* _toolWidget = CreateWidget<UCollectToolDurability>(_playerController, _widgetClass);
		if (!_toolWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("UUIPanelCollectSeat: Failed to create widget %d"), _i);
			continue;
		}

		// Visibility 확인 및 설정
		_toolWidget->SetVisibility(ESlateVisibility::Visible);

		// Spacer_Left 다음에 추가 (인덱스는 1부터 시작)
		UPanelSlot* _panelSlot = HorizontalBox->InsertChildAt(_spacerLeftIndex + 1 + _i, _toolWidget);
		UHorizontalBoxSlot* _slot = Cast<UHorizontalBoxSlot>(_panelSlot);
		if (_slot)
		{
			// 슬롯 크기를 Auto로 설정
			_slot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			_slot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
			_slot->SetHorizontalAlignment(HAlign_Fill);
			_slot->SetVerticalAlignment(VAlign_Fill);
		}

		_toolDurabilityWidgets.Add(_toolWidget);
	}

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
}

void UUIPanelCollectSeat::OnChangeDurability(UEventOnCollectToolDurability* Event)
{
	if (Event == nullptr)
		return;
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

	_toolDurabilityWidgets.Empty();
}