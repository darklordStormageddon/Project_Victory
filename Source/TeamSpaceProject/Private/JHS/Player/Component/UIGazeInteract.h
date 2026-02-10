// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "UIGazeInteract.generated.h"

class UCameraComponent;
class UInteractableButton;
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
	// 현재 포커스된 버튼을 클릭(입력 바인딩은 Pawn/Controller에서 이 함수를 호출하도록 구성)
	UFUNCTION(BlueprintCallable, Category = "UI|Gaze")
	void ClickFocused();

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

	TWeakObjectPtr<UInteractableButton> _focusedButton;

private:
	UCameraComponent* _ResolveCamera() const;
	UInteractableButton* _FindInteractableFromHit(const FHitResult& HitResult) const;
	void _UpdateFocus(UInteractableButton* NewButton);
};

