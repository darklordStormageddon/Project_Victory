// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

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
	UIPanelTurretMagReload UMETA(DisplayName = "UIPanelTurretMagReload"),
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

	NONE UMETA(DisplayName = "NONE"),
};

class CommonEnums
{
public:
	CommonEnums();
	~CommonEnums();

public:
	template <typename EnumType>
	static FString GetEnum2FString(EnumType InEnum)
	{
		static_assert(TIsEnum<EnumType>::Value, "EnumType must be an enum.");
	
		FString _enumName = TEXT("Unknown");
		if (UEnum* _enum = StaticEnum<EnumType>())
		{
			_enumName = _enum->GetNameStringByValue(static_cast<int64>(InEnum));
		}
	
		return _enumName;
	}

	static bool TryGetInteractType(FString InEnumName, E_INTERACT_TYPE& OutInteractType);

	static bool TryGetElementType(FString InEnumName, E_ELEMENT_TYPE& OutElementType);

	static bool TryGetAmmoType(FString InEnumName, E_AMMO_TYPE& OutAmmoType);
};
