// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Event/CommonEventBase.h"

#include "UIPanelCollectSeat.generated.h"

class UHorizontalBox;
class USpacer;
class UCollectToolDurability;

UCLASS()
class UUIPanelCollectSeat : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnChangeDurability;

	FDelegateHandle _eventHandleOnChangeTool;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> Spacer_Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> Spacer_Right;

	TMap<E_COLLECT_TOOL_TYPE, TObjectPtr<UCollectToolDurability>> _toolDurabilityItemMap;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CollectSeat")
	TSubclassOf<UCollectToolDurability> _toolDurabilityWidgetClass;

protected:
	void NativeOnInitialized() override;

	void OnOpen() override;

	void OnClose() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangeDurability(UEventOnChangeToolDurability* Event);

	void OnChangeTool(UEventOnChangeTool* Event);

private:
	void ClearDynamicWidgets();

	TObjectPtr<UCollectToolDurability> GetCollectToolItem(E_COLLECT_TOOL_TYPE CollectToolType);
};
