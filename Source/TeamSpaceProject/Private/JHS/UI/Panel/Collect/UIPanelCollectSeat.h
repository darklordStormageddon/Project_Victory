// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Event/CommonEventBase.h"

#include "UIPanelCollectSeat.generated.h"

UCLASS()
class UUIPanelCollectSeat : public UUIBase
{
	GENERATED_BODY()

public:
	void OnChangeDurability(UEventOnCollectToolDurability* Event);
};
