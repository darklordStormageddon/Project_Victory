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
class UScrollBox;
class USizeBox;
class UPlateContainer;

UCLASS()
class UUIPanelShop : public UUIBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_Category = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SB_ItemRow = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SB_PlateContainer = nullptr;

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPurchaseCategory> _purchaseCategoryClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPurchaseRow> _purchaseRowClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPlateContainer> _plateContainerClass;

	TMap<E_PURCHASE_CATEGORY, TObjectPtr<UPurchaseCategory>> _purchaseCategoryMap;

	UPROPERTY()
	TObjectPtr<UPlateContainer> _plateContainer = nullptr;

	TMap<int32, TObjectPtr<UPurchaseRow>> _purchaseRowMap;

	E_PURCHASE_CATEGORY _selectedCategory = E_PURCHASE_CATEGORY::SpaceShip;

	static constexpr int32 _maxPurchaseRowCount = 50;

protected:
	void NativeOnInitialized() override;

	void OnOpen() override;

private:
	void CreatePurchaseCategories();

	void SelectCategory(E_PURCHASE_CATEGORY Category);

	void UpdatePurchaseRow(E_PURCHASE_CATEGORY SelectedCategory);

	TArray<FPurchaseData> GetPurchaseDataArray(E_PURCHASE_CATEGORY SelectedCategory);

	bool TryGetPurchaseCategory(E_PURCHASE_CATEGORY Category, TObjectPtr<UPurchaseCategory>& OutPurchaseCategory);
};
