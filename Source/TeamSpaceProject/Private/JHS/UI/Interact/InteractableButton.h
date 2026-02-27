// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/Interact/InteractableUIBase.h"

#include "InteractableButton.generated.h"

class UButton;
class UImage;
class UTextBlock;

/**
 * Gaze(시선) 기반으로 포커스/클릭을 받을 수 있는 버튼 위젯
 * - 위젯 블루프린트에서 Button 이름을 반드시 "BTN_Button"으로 두고 바인딩하세요.
 */
UCLASS()
class UInteractableButton : public UInteractableUIBase
{
	GENERATED_BODY()

private:
	// BindWidget은 위젯 이름과 C++ 멤버명이 같아야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_Button = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_OnHover = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Label = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_BlockClick = nullptr;

protected:
	void NativeOnInitialized() override;

	void NativeConstruct() override;

	void OnChangeClickable(bool IsClickable) override;

	void OnHover() override;

	void OnUnhover() override;

	void OnClickEnter() override;

	void OnClickExit() override;

public:
	void InitializeButton(FString Label);

private:
	UFUNCTION()
	void OnButtonClicked();
};

