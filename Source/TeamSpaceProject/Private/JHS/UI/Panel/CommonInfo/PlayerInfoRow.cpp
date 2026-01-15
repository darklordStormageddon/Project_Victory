// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/CommonInfo/PlayerInfoRow.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "JHS/UI/UIBase.h"

void UPlayerInfoRow::InitializeRaw(int32 PlayerNumber)
{
	if (TXT_PlayerNumber)
	{
		const FString _label = FString::Printf(TEXT("%dP"), PlayerNumber);
		TXT_PlayerNumber->SetText(FText::FromString(_label));
	}
}

void UPlayerInfoRow::UpdatePlayerRadiationDose(FMaxCurrentData RadiationValue)
{
	UUIBase::SetProgressBarUI(RadiationValue.CurrentValue, RadiationValue.MaxValue, PROG_RadiationDose, nullptr);
}