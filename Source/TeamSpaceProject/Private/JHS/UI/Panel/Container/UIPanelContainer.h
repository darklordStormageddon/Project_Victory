// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "UIPanelContainer.generated.h"

class UUniformGridPanel;
class UContainerItemSlot;
class UEventOnChangeElementData;

UCLASS()
class UUIPanelContainer : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnChangeElementData;

	TMap<E_ELEMENT_TYPE, TObjectPtr<UContainerItemSlot>> _elementSlotMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> UG_Items;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Container")
	TSubclassOf<UContainerItemSlot> _itemSlotClass;

private:
	int32 _currentSlotCount = 0;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Container")
	float _slotSizeWidth = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Container")
	float _slotSizeHeight = 400.0f;

protected:
	virtual bool Initialize() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

	void OnOpen() override;

	void OnClose() override;

public:
	void OnChangeElementData(UEventOnChangeElementData* Event);

private:
	void SetSlotIndex(TObjectPtr<UContainerItemSlot> ItemSlot, int32 Index);

	TObjectPtr<UContainerItemSlot> CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType);

	void SortItemSlot();
};
