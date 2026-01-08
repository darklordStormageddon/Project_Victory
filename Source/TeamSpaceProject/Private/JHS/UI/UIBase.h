// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIBase.generated.h"

UENUM(BlueprintType)
enum class E_UI_TYPE : uint8
{
	// Panel
	UIPanelPlayerFPS = 0 UMETA(DisplayName = "UIPanelPlayerFPS"),
	UIPanelDriveSeat UMETA(DisplayName = "UIPanelDriveSeat"),
	UIPanel UMETA(DisplayName = "SpaceGarbage"),

	// Popup
	UIPopupCommon = 100 UMETA(DisplayName = "UIPopupCommon"),

	// System
	UISystemSetting = 200 UMETA(DisplayName = "UISystemSetting"),
};

/**
 * 
 */
UCLASS()
class UUIBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UUIBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual void OnOpen() { }

	virtual void OnClose() { }

	virtual void RegisterEvent() { }

	virtual void UnregisterEvent() { }

public:
	UPROPERTY(BlueprintReadOnly, Category = "UI|Base")
	E_UI_TYPE CurrentType;

	UFUNCTION(BlueprintPure, Category = "UI|Base")
	bool IsActivate() const { return _isActive && IsInViewport(); }

	void Open();

	void Close();

	void SetAsFirstSibling();

	void SetAsLastSibling();

private:
	bool _isInitialized = false;
	bool _isActive = false;
};
