// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Components/Image.h"
#include "JHS/GameControl/CommonEnums.h"

#include "UIPanelPlayerFPS.generated.h"

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
	GENERATED_BODY()

private:
	TMap<E_INTERACT_TYPE, TObjectPtr<UTexture2D>> _interacTextureMap;

private:
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Interact;

protected:
	void NativeConstruct() override;

	//void RegisterEvent() override;

	//void UnregisterEvent() override;

public:
	void InitializeUI();

	void ChangeInteractable(E_INTERACT_TYPE InteractType);
};
