// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/ShopManager.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"

// Sets default values for this component's properties
UShopManager::UShopManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UShopManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UShopManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UShopManager::InitializeShop(TObjectPtr<UContainerStateGroup> ContainerStateGroup)
{
	_containerStateGroup = ContainerStateGroup;
}

bool UShopManager::TryPurchase(FPurchaseData* PurchaseData)
{
	FPurchaseData _purchaseData = *PurchaseData;

	// 레벨 비교
	FMaxCurrentData* _level = &_purchaseData.Level;
	if (_level->MaxValue != -1 && _level->CurrentValue >= _level->MaxValue)
		return false;

	if (_containerStateGroup == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UShopManager::TryPurchase: ContainerStateGroup is null"));
		return false;
	}

	// 가격 비교
	if (!_containerStateGroup->TryConsumeDollar((int32)_purchaseData.PurchaseDollar))
		return false;

	// 레벨 증가
	_level->CurrentValue++;
	if (_level->CurrentValue >= _level->MaxValue)
	{
		_level->CurrentValue = _level->MaxValue;
		_purchaseData.IsPurchaseable = false;

		_purchaseData.PurchaseDollar = 0.0f;
	}
	else
	{
		// 값 증가
		_purchaseData.Value.MaxValue = CalculateValue(_purchaseData.InitValue, _purchaseData.IncreasePerValue, _level->CurrentValue);
		_purchaseData.Value.CurrentValue = _purchaseData.Value.MaxValue;

		// 가격 증가
		_purchaseData.PurchaseDollar = (int32)CalculateValue(_purchaseData.InitDollar, _purchaseData.IncreasePerDollar, _level->CurrentValue);
	}

	*PurchaseData = _purchaseData;
	return true;
}

float UShopManager::CalculateValue(float InitValue, float IncreasePerValue, int32 Level)
{
	return InitValue * (1.0f + IncreasePerValue * 0.01f * --Level);
}