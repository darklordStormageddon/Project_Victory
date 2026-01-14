// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Delegates/Delegate.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "JHS/Event/CommonEventBase.h"

#include "UIPanelTurretSeat.generated.h"

/**
 * 
 */
UCLASS()
class UUIPanelTurretSeat : public UUIBase
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnChangeTurret;
	FDelegateHandle _eventHandleOnChangeTurretAmmo;

#pragma region Main Turret
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_LeftAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_LeftAmmo;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> _mainAmmoMID;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorMax = FColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorZero = FColor::Red;

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangeTurret(UEventOnChangeTurretData* Event);

	void OnChangeTurretAmmo(UEventOnChangeTurretAmmo* Event);

private:
	TObjectPtr<UMaterialInstanceDynamic> GetMainTurretMaterial();
};
