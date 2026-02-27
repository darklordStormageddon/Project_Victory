// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Event/CommonEventBase.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "UIPanelTurretMagReload.generated.h"

UCLASS()
class UUIPanelTurretMagReload : public UUIBase
{
	GENERATED_BODY()
	
private:
	FDelegateHandle _eventHandleOnChangeTurret;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_TurretPosition;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_TurretMag;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_TurretMag;

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangeTurret(UEventOnChangeTurretData* Event);

	void InitializeMagReload();
};
