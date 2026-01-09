// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Delegates/Delegate.h"
#include "UIPanelDriveSeat.generated.h"

class UEventOnChangeSpaceShipData;

/**
 * 
 */
UCLASS()
class UUIPanelDriveSeat : public UUIBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Plate_SpaceShipInfo;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_SpaceShipShield;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_SpaceShipShield;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_SpaceShipHP;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_SpaceShipHP;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_SpaceShipFuel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_SpaceShipFuel;

	FDelegateHandle _eventHandle;

protected:
	void OnOpen() override;

	void OnClose() override;

public:
	void OnChangeSpaceShipData(UEventOnChangeSpaceShipData* Event);

	void UpdateSpaceShipUI(float CurrentValue, float MaxValue, UTextBlock* TextBlock, UProgressBar* ProgressBar);
};
