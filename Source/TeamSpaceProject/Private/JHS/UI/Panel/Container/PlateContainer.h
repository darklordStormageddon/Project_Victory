// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PlateContainer.generated.h"

class UContainerStateGroup;
class UInteractableScrollBox;
class UContainerItemSlot;
class UTextBlock;
class UEventOnChangeElementData;
class UEventOnChangeOwnedDollar;
class UEventOnChangeGoalDollar;

UCLASS()
class UPlateContainer : public UUserWidget
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnChangeElementData;

	FDelegateHandle _eventHandleOnChangeOwnedDollar;

	FDelegateHandle _eventHandleOnChangeGoalDollar;

	TMap<E_ELEMENT_TYPE, TObjectPtr<UContainerItemSlot>> _elementSlotMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractableScrollBox> SB_Items;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_CumulativePrice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_OwnedDollar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_ExpectDollar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_GoalDollar;

private:
	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer|ItemSlot")
	TSubclassOf<UContainerItemSlot> _itemSlotClass;

	UPROPERTY(EditAnywhere, Category = "PlateContainer|Expect Dollar Color")
	FLinearColor _lessExpectDollarColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "PlateContainer|Expect Dollar Color")
	FLinearColor _overExpectDollarColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "PlateContainer|Expect Dollar Effect")
	float _effectUpdateTime = 1.0f;

private:
	UPROPERTY()
	int32 _currentSlotCount = 0;

	FTimerHandle _timerHandle;

	UPROPERTY()
	float _currentEffectUpdateTime = 0.0f;

	UPROPERTY()
	int32 _goalDollar = 0;

	UPROPERTY()
	float _targetCumulativePrice = 0;

	UPROPERTY()
	float _currentCumulativePrice = 0;

	UPROPERTY()
	float _targetOwnedDollar = 0;

	UPROPERTY()
	float _currentOwnedDollar = 0;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer")
	float _slotSizeWidth = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "PlateContainer")
	float _slotSizeHeight = 400.0f;

protected:
	void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:
	void OnChangeElementData(UEventOnChangeElementData* Event);

	void OnChangeOwnedDollar(UEventOnChangeOwnedDollar* Event);

	void OnChangeGoalDollar(UEventOnChangeGoalDollar* Event);

	void UpdateDollar();

	TObjectPtr<UContainerItemSlot> CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType);

	void SortItemSlot();

	void ClearTimer();

	void UpdateText();

	bool UpdateValue(float* CurrentValue, float TargetValue, TObjectPtr<UTextBlock> TextBlock);
};