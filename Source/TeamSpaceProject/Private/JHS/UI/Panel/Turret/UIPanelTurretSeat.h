// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Event/CommonEventBase.h"

#include "UIPanelTurretSeat.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTurretStateGroup;
class UCircleProgressBar;

UCLASS()
class UUIPanelTurretSeat : public UUIBase
{
	GENERATED_BODY()

public:
	UUIPanelTurretSeat(const FObjectInitializer& ObjectInitializer);

private:
	FDelegateHandle _eventHandleOnChangeTurret;

	UPROPERTY()
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;

#pragma region Main Turret
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_LeftAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_LeftAmmo;

	UPROPERTY(VisibleAnywhere, Category = "TurretSeat|Component")
	TObjectPtr<UCircleProgressBar> _circleProgressBar = nullptr;
#pragma endregion Main Turret

#pragma region Left Turret
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_LeftTurret;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_LeftTurretAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_LeftTurretAmmo;
#pragma endregion Left Turret

#pragma region Right Turret
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_RightTurret;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PROG_RightTurretAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_RightTurretAmmo;
#pragma endregion Right Turret

protected:
	UPROPERTY(EditDefaultsOnly, Category = "TurretSeat|Generator")
	TObjectPtr<UTexture2D> _initTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "TurretSeat|Generator")
	bool _isClockWise = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorMax = FColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorZero = FColor::Red;

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

	void RegisterEvent() override;

	void UnregisterEvent() override;

private:
	void OnChangeTurret(UEventOnChangeTurretData* Event);
};