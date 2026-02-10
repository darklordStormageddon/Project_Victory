// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"

#include "UIPanelContainer.generated.h"

class USizeBox;
class UPlateContainer;

UCLASS()
class UUIPanelContainer : public UUIBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SB_PlateContainer;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Container")
	TSubclassOf<UPlateContainer> _plateContainerClass;

private:
	UPROPERTY()
	TObjectPtr<UPlateContainer> _plateContainer = nullptr;

protected:
	void NativeOnInitialized() override;

	void OnOpen() override;
};
