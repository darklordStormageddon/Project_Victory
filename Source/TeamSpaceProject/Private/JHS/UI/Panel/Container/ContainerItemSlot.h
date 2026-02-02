// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "ContainerItemSlot.generated.h"

class USizeBox;
class UImage;
class UTextBlock;

UCLASS()
class UContainerItemSlot : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> Plate_Slot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ItemPrice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ItemAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_TotalPrice;

public:
	void UpdateItemInfo(FElementData ElementData);
};
