// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "UIPanelShop.generated.h"

class AJHSGameState;
class UPurchaseCategory;
class UPurchaseRow;
class UHorizontalBox;
class UInteractableScrollBox;
class USizeBox;
class UPlateContainer;
class UInteractableButton;
class UEventOnPurchase;

UCLASS()
class UUIPanelShop : public UUIBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_Category = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractableScrollBox> SB_ScrollBox = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SB_PlateContainer = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractableButton> BTN_SaleElement = nullptr;

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPurchaseCategory> _purchaseCategoryClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPurchaseRow> _purchaseRowClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPlateContainer> _plateContainerClass;

	UPROPERTY()
	TMap<E_PURCHASE_CATEGORY, TObjectPtr<UPurchaseCategory>> _purchaseCategoryMap;

	UPROPERTY()
	TObjectPtr<UPlateContainer> _plateContainer = nullptr;

	UPROPERTY()
	TMap<int32, TObjectPtr<UPurchaseRow>> _purchaseRowMap;

	UPROPERTY()
	E_PURCHASE_CATEGORY _selectedCategory = E_PURCHASE_CATEGORY::SpaceShip;

	FDelegateHandle _eventHandleOnPurchase;

protected:
	void NativeOnInitialized() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

	void OnOpen() override;

public:
	void SelectCategory(E_PURCHASE_CATEGORY Category);

private:
	void CreatePurchaseCategories();

	void UpdatePurchaseRow(E_PURCHASE_CATEGORY SelectedCategory, TObjectPtr<UPurchaseCategory> CategoryButton);

	TArray<FPurchaseData*> GetPurchaseDataArray(E_PURCHASE_CATEGORY SelectedCategory);

	bool TryGetPurchaseCategory(E_PURCHASE_CATEGORY Category, TObjectPtr<UPurchaseCategory>& OutPurchaseCategory);

	void OnChangeSpaceShipData(UEventOnPurchase* Event);

	// BTN_SaleElement
private:
	UFUNCTION()
	void OnClickSaleElement();

	void AddElement();

	FTimerHandle _timerHandle;

	int32 _elementIndex = -1;
};
