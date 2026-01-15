// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateData.h"

#include "PlayerInfoRow.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class UPlayerInfoRow : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_PlayerNumber;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PROG_RadiationDose;

public:
	void InitializeRaw(int32 PlayerNumber);

	void UpdatePlayerRadiationDose(FMaxCurrentData RadiationValue);
};
