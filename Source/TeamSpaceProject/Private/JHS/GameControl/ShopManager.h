// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "ShopManager.generated.h"

USTRUCT(BlueprintType)
struct FPurchaseDataGroup
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_PURCHASE_CATEGORY PurchaseCategory = E_PURCHASE_CATEGORY::END;

	UPROPERTY()
	TMap<int8, FPurchaseData> PurchaseDataMap;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UShopManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShopManager();

private:
	UPROPERTY()
	TMap<E_PURCHASE_CATEGORY, FPurchaseDataGroup> _purchaseDataGroupMap;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeShop();
};
