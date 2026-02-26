// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/CommonEnums.h"

#include "UIBase.generated.h"

class UEventManager;
class UTextBlock;
class UProgressBar;
class UWidgetComponent;

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

	TWeakObjectPtr<UWidgetComponent> _hostWidgetComponent;

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

	/** 월드 공간 UI일 때 호스트 WidgetComponent 설정. 충돌은 기본 비활성, Open() 시 UInteractableButton 존재하면 활성화 */
	void SetHostWidgetComponent(UWidgetComponent* InComponent);

public:
	static void SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar, UTextBlock* TextBlock, bool IsOnlyCurrentText);

	static void SetProgressBarUI(float CurrentValue, float MaxValue, UProgressBar* ProgressBar);

protected:
	TObjectPtr<UEventManager> GetEventManager();

	/** 자식 중 UInteractableButton이 하나라도 있으면 호스트 충돌 활성화(QueryOnly) */
	void UpdateHostCollision();
};
