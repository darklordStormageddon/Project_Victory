// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Components/CanvasPanel.h"

#include "UIPanelPlayerFPS.generated.h"

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Plate_Interact;

	bool _isInteractable = false;
	
public:
	bool GetIsInteractable() { return _isInteractable; }

public:
	void InitializeUI();

	void ChangeInteractable(bool IsInteract);
};
