// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "InteractableUIBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableButtonEvent);

UCLASS()
class UInteractableUIBase : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void OnHover() {}

	virtual void OnUnhover() {}

	virtual void OnClickEnter() {}

	virtual void OnClickExit() {}

public:
	/** ??? ??? ???? ?? (InteractableScrollBox ??? ?????) */
	virtual bool WantsScrollInput() const { return false; }

	/** ?? ?? ??? ?? ??? ?? ?? (DeltaY: ??=??, ??=?) */
	virtual void ProcessScrollInput(float DeltaY) {}

public:
	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void Focus();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void Unfocus();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void ClickEnter();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void ClickExit();

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "UI|UInteractableUIBase")
	void OnHovered();
	virtual void OnHovered_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "UI|UInteractableUIBase")
	void OnUnhovered();
	virtual void OnUnhovered_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "UI|UInteractableUIBase")
	void OnClickedEnter();
	virtual void OnClickedEnter_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "UI|UInteractableUIBase")
	void OnClickedExit();
	virtual void OnClickedExit_Implementation() {}

public:
	// ?????? ????? ?????? ????(???? ??????)
	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Hovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Unhovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent ClickedEnter;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent ClickedExit;

	/** ??? ??: ClickEnter ? ?? ??????? */
	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Clicked;
};
