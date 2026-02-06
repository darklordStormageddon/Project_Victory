// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "UIPanelShop.generated.h"

class UPurchaseCategory;
class UHorizontalBox;

UCLASS()
class UUIPanelShop : public UUIBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_Category = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPurchaseCategory> _purchaseCategoryClass;

	TMap<E_PURCHASE_CATEGORY, TObjectPtr<UPurchaseCategory>> _purchaseCategoryMap;

	E_PURCHASE_CATEGORY _selectedCategory = E_PURCHASE_CATEGORY::SpaceShip;

protected:
	void NativeOnInitialized() override;

	void OnOpen() override;

private:
	void CreatePurchaseCategories();

	void SelectCategory(E_PURCHASE_CATEGORY Category);

	bool TryGetPurchaseCategory(E_PURCHASE_CATEGORY Category, TObjectPtr<UPurchaseCategory>& OutPurchaseCategory);
};
