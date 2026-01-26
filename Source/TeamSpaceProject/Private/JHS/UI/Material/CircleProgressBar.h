// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "CircleProgressBar.generated.h"

class UImage;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCircleProgressBar : public UActorComponent
{
	GENERATED_BODY()

private:
	// 직렬화
	UPROPERTY()
	TObjectPtr<UObject> _inOuter = nullptr;

	UPROPERTY()
	TObjectPtr<UImage> _image = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> _MID = nullptr;

	// const
	// C:/Users/jhsro/source/repos/Unreal/TeamSpace/Content/Main/PS_JHS/Resource/Materials/MT_CircleProgressBar.MT_CircleProgressBar
	const FName PARAMETER_TEXTURE = TEXT("IconTex");
	const FName PARAMETER_IS_CLOCK_WISE = TEXT("IsClockWise");
	const FName PARAMETER_PROGRESS = TEXT("Progress");
	const FName PARAMETER_TINT = TEXT("Tint");

public:
	UCircleProgressBar();

public:
	void InitializeCircleProgressBar(TObjectPtr<UObject> InOuter, TObjectPtr<UImage> Image, TObjectPtr<UTexture2D> Texture, bool IsClockWise);

	void SetTexture(TObjectPtr<UTexture2D> Texture);

	void SetClockWise(bool IsClockWise);

	void SetProgress(float Progress);

	void SetTint(FLinearColor Tint);
};
