// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/Interact/InteractableUIBase.h"

#include "InteractableScrollBox.generated.h"

class UScrollBox;
class UImage;

/**
 * 시선(Gaze) 기반으로 포커스/클릭을 받을 수 있는 스크롤박스 위젯
 * - 위젯 블루프린트에서 ScrollBox 이름을 반드시 "SCRL_ScrollBox"로 두고 바인딩하세요.
 * - ClickEnter 호출 시마다 _bScrollMode 토글 (Release/ClickExit는 상태 변경 없음)
 * - _bScrollMode = true일 때 마우스 휠로 스크롤 조작
 */
UCLASS()
class UInteractableScrollBox : public UInteractableUIBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SCRL_ScrollBox = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_OnHover = nullptr;

	/** 마우스 휠당 스크롤 감도 (1.0=기본, GetGlobalScrollAmount 기준 배율) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|InteractableScrollBox", meta = (AllowPrivateAccess = "true"))
	float _scrollSensitivity = 1.0f;

	bool _isScrollMode = false;

public:
	float GetScrollSensitivity() const { return _scrollSensitivity; }
	void SetScrollSensitivity(float Value) { _scrollSensitivity = Value; }

protected:
	void NativeOnInitialized() override;

	void NativeConstruct() override;

	void OnHover() override;

	void OnUnhover() override;

	void OnClickEnter() override;

	void OnClickExit() override;

	bool WantsScrollInput() const override;

	void ProcessScrollInput(float DeltaY) override;

public:
	void AddChild(UWidget* Content);

	void RemoveChild(UWidget* Content);

	void ClearChildren();

private:
	void ChangeScrollMode(bool IsScrollMode);
};
