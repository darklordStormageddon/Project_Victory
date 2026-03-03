// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"

#include "UIPanelCommonInfo.generated.h"

class UHorizontalBox;
class UImage;
class UProgressBar;
class UEventOnChangePlayerRadiation;

UCLASS()
class UUIPanelCommonInfo : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> GRP_RadiationDose;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_RadiationDose;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PROG_RadiationDose;

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void InitializeUI(TObjectPtr<UTexture2D> RadiationImage);

private:
	void OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event);
};
