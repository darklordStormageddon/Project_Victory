// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CollectToolDurability.generated.h"

class UImage;
class UCircleProgressBar;

UCLASS()
class UCollectToolDurability : public UUserWidget
{
	GENERATED_BODY()

public:
	UCollectToolDurability(const FObjectInitializer& ObjectInitializer);
	
private:
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Durability;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Icon;

	FDelegateHandle _eventHandleOnChangeTurret;

	UPROPERTY(VisibleAnywhere, Category = "CollectTool|Component")
	TObjectPtr<UCircleProgressBar> _circleProgressBar = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "CollectTool|Generator")
	TObjectPtr<UTexture2D> _initTexture = nullptr;

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

public:
	void SetDurabilityProgress(float Progress);
};
