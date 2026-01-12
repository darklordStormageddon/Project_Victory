// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Delegates/Delegate.h"
#include "Components/VerticalBox.h"

#include "UIPanelCommonInfo.generated.h"

class UPlayerInfoRow;
class UEventOnChangePlayerRadiation;

UCLASS()
class UUIPanelCommonInfo : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandle;

#pragma region Player Radiation Dose
	const FString TEXT_HEADER = "TXT_RadiationDoseP";

	const FString PROG_HEADER = "PROG_RadiationDoseP";

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> Plate_RadiationDose;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerInfoRow> WBP_PlayerInfoRow;

	TMap<int32, TObjectPtr<UPlayerInfoRow>> _playerRadiationDoseMap;
#pragma endregion Player Radiation Dose

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event);

	void InitializeUI();

public:
	void BuildRows(int32 InPlayerCount);

private:
	void ClearDynamicRows();
};
