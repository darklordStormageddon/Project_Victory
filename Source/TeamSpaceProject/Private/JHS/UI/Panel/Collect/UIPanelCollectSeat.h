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
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> Spacer_Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> Spacer_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCollectToolDurability> WBP_CollectToolDurability;

	TArray<TObjectPtr<UCollectToolDurability>> _toolDurabilityWidgets;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CollectSeat")
	int32 _toolCount = 3;

protected:
	void OnOpen() override;

	void OnClose() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void InitializeCollectSeat(int32 ToolCount);

	void OnChangeDurability(UEventOnCollectToolDurability* Event);

private:
	void ClearDynamicWidgets();
};
