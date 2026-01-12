// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Delegates/Delegate.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "JHS/GameControl/CommonEnums.h"

#include "UIPanelPlayerFPS.generated.h"

class UPlayerInfoRow;
class UEventOnChangePlayerRadiation;

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandle;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Plate_Interact;

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

public:
	void InitializeUI();

	void ChangeInteractable(E_INTERACT_TYPE InteractType);

public:
	void BuildRows(int32 InPlayerCount);

private:
	void ClearDynamicRows();
};
