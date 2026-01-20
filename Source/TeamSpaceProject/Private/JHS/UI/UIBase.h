// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/CommonEnums.h"

#include "UIBase.generated.h"

class UEventManager;
class UTextBlock;
class UProgressBar;

UCLASS()
class UUIBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UUIBase(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY()
	TObjectPtr<UEventManager> _cachedEventManager = nullptr;

	bool _isInitialized = false;

	bool _isActive = false;

protected:
	virtual void NativeOnInitialized() override;

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

public:
	static void SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar, UTextBlock* TextBlock, bool IsOnlyCurrentText);

protected:
	TObjectPtr<UEventManager> GetEventManager();
};
