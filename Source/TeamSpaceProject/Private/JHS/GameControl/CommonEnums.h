// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class E_UI_TYPE : uint8
{
	NONE = 255 UMETA(DisplayName = "NONE"),

	// Panel
	UIPanelCommonInfo = 0 UMETA(DisplayName = "UIPanelCommonInfo"),
	UIPanelPlayerFPS UMETA(DisplayName = "UIPanelPlayerFPS"),
	UIPanelDriveSeat UMETA(DisplayName = "UIPanelDriveSeat"),
	UIPanelCollectSeat UMETA(DisplayName = "UIPanelCollectSeat"),
	UIPanelTurretSeat UMETA(DisplayName = "UIPanelTurretSeat"),
	UIPanelContainer UMETA(DisplayName = "UIPanelContainer"),

	// Popup
	UIPopupCommon = 100 UMETA(DisplayName = "UIPopupCommon"),

	// System
	UISystemSetting = 200 UMETA(DisplayName = "UISystemSetting"),
};

UENUM(BlueprintType)
enum class E_INTERACT_TYPE : uint8
{
	// Panel
	Idle = 0 UMETA(DisplayName = "Idle"),
	Seat UMETA(DisplayName = "Seat"),
	DumpThrow UMETA(DisplayName = "DumpThrow"),
};

class CommonEnums
{
public:
	CommonEnums();
	~CommonEnums();

public:
	static FString GetFStringInteractEnum(E_INTERACT_TYPE InteractType);
};
