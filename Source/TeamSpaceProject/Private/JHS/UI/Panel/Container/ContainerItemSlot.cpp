// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridSlot.h"

void UContainerItemSlot::InitializeSlot(float slotSizeWidth, float slotSizeHeight)
{
	_slot = Cast<UUniformGridSlot>(Slot);
	if (_slot != nullptr)
	{
		_slot->SetHorizontalAlignment(HAlign_Fill);
		_slot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UContainerItemSlot::UpdateItemInfo(FString ItemName, int32 Amount)
{
	if (TXT_ItemName != nullptr)
	{
		TXT_ItemName->SetText(FText::FromString(ItemName));
	}

	if (TXT_ItemAmount != nullptr)
	{
		TXT_ItemAmount->SetText(FText::AsNumber(Amount));
	}
}