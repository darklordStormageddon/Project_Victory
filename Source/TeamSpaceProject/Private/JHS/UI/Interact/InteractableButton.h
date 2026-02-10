// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "InteractableButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableButtonEvent);

/**
 * Gaze(시선) 기반으로 포커스/클릭을 받을 수 있는 버튼 위젯
 * - 위젯 블루프린트에서 Button 이름을 반드시 "BTN_Button"으로 두고 바인딩하세요.
 */
UCLASS()
class UInteractableButton : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	// BindWidget은 위젯 이름과 C++ 멤버명이 같아야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_Button = nullptr;

private:
	UFUNCTION()
	void HandleHovered();

	UFUNCTION()
	void HandleUnhovered();

	UFUNCTION()
	void HandleClicked();

public:
	// 외부(예: UIGazeInteract)에서 호출할 API
	UFUNCTION(BlueprintCallable, Category = "UI|InteractableButton")
	void Focus();

	UFUNCTION(BlueprintCallable, Category = "UI|InteractableButton")
	void Unfocus();

	UFUNCTION(BlueprintCallable, Category = "UI|InteractableButton")
	void Click();

	// 위젯 내부 연출용(블루프린트에서 구현/오버라이드 가능)
	UFUNCTION(BlueprintNativeEvent, Category = "UI|InteractableButton")
	void OnHovered();
	virtual void OnHovered_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "UI|InteractableButton")
	void OnUnhovered();
	virtual void OnUnhovered_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "UI|InteractableButton")
	void OnClicked();
	virtual void OnClicked_Implementation();

public:
	// 외부에서 바인딩 가능한 이벤트(게임 로직용)
	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Hovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Unhovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Clicked;
};

