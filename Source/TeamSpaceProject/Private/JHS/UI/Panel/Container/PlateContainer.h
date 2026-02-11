// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PlateContainer.generated.h"

class UContainerStateGroup;
class UScrollBox;
class UContainerItemSlot;
class UTextBlock;
class UEventOnChangeElementData;

UCLASS()
class UPlateContainer : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnChangeElementData;

	TMap<E_ELEMENT_TYPE, TObjectPtr<UContainerItemSlot>> _elementSlotMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SB_Items;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_CumulativePrice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_OwnedDollar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ExpectDollar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_GoalDollar;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer|ItemSlot")
	TSubclassOf<UContainerItemSlot> _itemSlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlateContainer|Expect Dollar Color")
	FLinearColor _lessExpectDollarColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlateContainer|Expect Dollar Color")
	FLinearColor _overExpectDollarColor = FColor::Green;

private:
	int32 _currentSlotCount = 0;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer")
	float _slotSizeWidth = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer")
	float _slotSizeHeight = 400.0f;

protected:
	void NativeOnInitialized() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

	void OnOpen() override;

	void OnClose() override;

public:
	void OnChangeElementData(UEventOnChangeElementData* Event);

private:
	TObjectPtr<UContainerItemSlot> CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType);

	void SortItemSlot();
};
