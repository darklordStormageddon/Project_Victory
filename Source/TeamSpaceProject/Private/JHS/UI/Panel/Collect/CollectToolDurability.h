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
	
private:
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Durability;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Icon;

	FDelegateHandle _eventHandleOnChangeTurret;

	UPROPERTY()
	TObjectPtr<UCircleProgressBar> _circleProgressBar = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "TurretSeat|Generator")
	TObjectPtr<UTexture2D> _initTexture = nullptr;

protected:
	virtual void NativeOnInitialized() override;
};
