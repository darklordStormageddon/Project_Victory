// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/UIBase.h"
#include "Components/Image.h"

#include "UIPanelPlayerFPS.generated.h"

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
	GENERATED_BODY()

private:
	TMap<E_INTERACT_TYPE, TObjectPtr<UTexture2D>> _interactTextureMap;

	FDelegateHandle _eventHandleOnChangeInteractType;

private:
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Interact;

protected:
	virtual void NativeOnInitialized() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

private:
	void OnChangeInteractType(UEventOnChangeInteractType* Event);

	void ChangeInteractable(E_INTERACT_TYPE InteractType);
};
