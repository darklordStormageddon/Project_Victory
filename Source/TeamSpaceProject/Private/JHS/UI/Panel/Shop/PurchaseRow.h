// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PurchaseRow.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class UPurchaseRow : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Icon = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Description = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_CurrentValue = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_NextValue = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_PurchaseDollar = nullptr;
	
public:
	void UpdateRow(FPurchaseData PurchaseData);
};
