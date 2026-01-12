// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Components/CanvasPanel.h"
#include "JHS/GameControl/CommonEnums.h"

#include "UIPanelPlayerFPS.generated.h"

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Plate_Interact;

protected:
	//void RegisterEvent() override;

	//void UnregisterEvent() override;

public:
	void InitializeUI();

	void ChangeInteractable(E_INTERACT_TYPE InteractType);
};
