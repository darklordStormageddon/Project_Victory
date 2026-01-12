// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Delegates/Delegate.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
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
	FDelegateHandle _eventHandle;

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

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangeSpaceShipData(UEventOnChangeSpaceShipData* Event);
};
