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

private:
	UPROPERTY()
	bool _isClickable = true;

public:
	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Hovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent Unhovered;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent ClickedEnter;

	UPROPERTY(BlueprintAssignable, Category = "UI|InteractableButton")
	FInteractableButtonEvent ClickedExit;
	
protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void OnChangeClickable(bool IsClickable) {}

	virtual void OnHover() {}

	virtual void OnUnhover() {}

	virtual void OnClickEnter() {}

	virtual void OnClickExit() {}

public:
	virtual bool WantsScrollInput() const { return false; }
	
	virtual void ProcessScrollInput(float DeltaY) {}

public:
	UFUNCTION()
	void SetClickable(bool IsClickable);

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void Focus();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void Unfocus();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void ClickEnter();

	UFUNCTION(BlueprintCallable, Category = "UI|UInteractableUIBase")
	void ClickExit();
};
