// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/UI/UIBase.h"
#include "Delegates/Delegate.h"
#include "Components/Image.h"
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
	FDelegateHandle _eventHandle;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_LeftAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_LeftAmmo;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> _leftAmmoMID;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorMax = FColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretSeat")
	FLinearColor _leftAmmoColorZero = FColor::Red;

protected:
	void RegisterEvent() override;

	void UnregisterEvent() override;

public:
	void OnChangeTurretAmmo(UEventOnChangeTurretAmmo* Event);
};
