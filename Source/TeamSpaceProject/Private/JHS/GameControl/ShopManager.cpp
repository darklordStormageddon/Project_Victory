// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/ShopManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
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
	if (_containerStateGroup == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UShopManager::TryPurchase: ContainerStateGroup is null"));
		return false;
	}

	FPurchaseData _purchaseData = *PurchaseData;
	FMaxCurrentData* _level = &_purchaseData.Level;

	// 가격 비교
	if (!_containerStateGroup->TryConsumeDollar((int32)_purchaseData.PurchaseDollar))
		return false;

	// 레벨 증가
	if (_level->MaxValue == -1 || _level->CurrentValue < _level->MaxValue)
	{
		_level->CurrentValue++;
	}

	if (_level->MaxValue != -1 && _level->CurrentValue >= _level->MaxValue)
	{
		_level->CurrentValue = _level->MaxValue;
		_purchaseData.PurchaseDollar = 0.0f;
		_purchaseData.IsPurchaseable = false;
	}
	else
	{
		// 가격 증가
		_purchaseData.PurchaseDollar = (int32)CalculateValue(_purchaseData.InitDollar, _purchaseData.IncreasePerDollar, _level->CurrentValue);
	}

	// 값 증가
	_purchaseData.Value.MaxValue = CalculateValue(_purchaseData.InitValue, _purchaseData.IncreasePerValue, _level->CurrentValue);
	_purchaseData.Value.CurrentValue = _purchaseData.Value.MaxValue;

	*PurchaseData = _purchaseData;

	UEventOnPurchase* _event = NewObject<UEventOnPurchase>(this);
	if (_event == nullptr)
		return true;

	UEventManager::ExecuteEvent<UEventOnPurchase>(_event);
	return true;
}

float UShopManager::CalculateValue(float InitValue, float IncreasePerValue, int32 Level)
{
	const float _increasePer = (100.0f + IncreasePerValue * Level) * 0.01f;
	return InitValue * _increasePer;
}