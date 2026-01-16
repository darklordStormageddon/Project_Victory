// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "Components/UniformGridSlot.h"

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

	UPROPERTY()
	TObjectPtr<UUniformGridSlot> _slot = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ItemAmount;

public:
	TObjectPtr<UUniformGridSlot> GetSlot() { return _slot; }

public:
	void InitializeSlot(float slotSizeWidth, float slotSizeHeight);

	void UpdateItemInfo(FString ItemName, int32 Amount);
};
