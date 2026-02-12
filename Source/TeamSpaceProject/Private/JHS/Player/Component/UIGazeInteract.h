// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "UIGazeInteract.generated.h"

class UCameraComponent;
class UInteractableUIBase;
struct FHitResult;

/**
 * Pawn 블루프린트에 부착해서 사용하는 시선 기반 UI 상호작용 컴포넌트
 * - 카메라 전방으로 LineTrace를 쏘고, 히트된 WidgetComponent의 위젯이 UInteractableButton이면 포커스/클릭을 호출합니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UIGazeInteract : public UActorComponent
{
	GENERATED_BODY()

public:
	UIGazeInteract(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 클릭 입력 시작 (Press 시 호출 - 입력 바인딩은 Pawn/Controller에서 구성)
	UFUNCTION(BlueprintCallable, Category = "UI|Gaze")
	void ClickEnterFocused();

	// 클릭 입력 종료 (Release 시 호출)
	UFUNCTION(BlueprintCallable, Category = "UI|Gaze")
	void ClickExitFocused();

	// 구버전 호환: 한 번 호출 시 ClickEnter + ClickExit 순차 호출 (바인딩을 ClickEnterFocused/ClickExitFocused로 분리 권장)
	UFUNCTION(BlueprintCallable, Category = "UI|Gaze", meta = (DeprecationMessage = "ClickEnterFocused와 ClickExitFocused로 분리 바인딩하세요"))
	void ClickFocused();

	/** 블루프린트에서 IA_Input(마우스 휠) 바인딩 시 호출. Delta를 InteractableUIBase::ProcessScrollInput으로 전달. Tick에서도 MouseWheelAxis를 읽어 자동 연동 */
	UFUNCTION(BlueprintCallable, Category = "UI|Gaze")
	void ProcessMouseWheelInput(float Delta);

	UFUNCTION(BlueprintCallable, Category = "UI|Gaze")
	void SetEnabled(bool bEnabled);

private:
	// ---- Serialized settings (Editor 노출) ----
	UPROPERTY(EditAnywhere, Category = "UI|Gaze", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float _distance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "UI|Gaze", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> _traceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "UI|Gaze", meta = (AllowPrivateAccess = "true"))
	bool _drawDebug = false;

private:
	// ---- Runtime state ----
	bool _enabled = true;

	TWeakObjectPtr<UInteractableUIBase> _focusedInteractableUI;

	TWeakObjectPtr<UInteractableUIBase> _clickedInteractableUI;

	float _lastHitLocationY = 0.0f;

	bool _lastHitValid = false;

private:
	UCameraComponent* _ResolveCamera() const;
	UInteractableUIBase* _FindInteractableFromHit(const FHitResult& HitResult) const;
	void _UpdateFocus(UInteractableUIBase* NewInteractableUI);
};

