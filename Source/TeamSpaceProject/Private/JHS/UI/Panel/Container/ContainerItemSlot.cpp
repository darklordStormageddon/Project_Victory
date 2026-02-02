// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UContainerItemSlot::UpdateItemInfo(FElementData ElementData)
{
	IMG_ItemIcon->SetBrushFromSoftTexture(ElementData.ElementImage);
	TXT_ItemName->SetText(FText::FromString(ElementData.KRName));
	TXT_ItemPrice->SetText(FText::AsNumber(ElementData.Price));
	TXT_ItemAmount->SetText(FText::AsNumber(ElementData.Amount));
	TXT_TotalPrice->SetText(FText::AsNumber(ElementData.Price * ElementData.Amount));
}