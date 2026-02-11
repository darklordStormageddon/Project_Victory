// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/UI/UIBase.h"

#include "InteracterComponent.generated.h"

class UUIManager;
class UUIPanelPlayerFPS;
class UInteractableComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInteracterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteracterComponent();

private:
	const E_UI_TYPE _playerUI = E_UI_TYPE::UIPanelPlayerFPS;

	UPROPERTY()
	TObjectPtr<UUIPanelPlayerFPS> _uiPanelPlayer = nullptr;

	UPROPERTY()
	TObjectPtr<UInteractableComponent> _interactable = nullptr;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void OnInteractable(TObjectPtr<UInteractableComponent> Interactable, E_INTERACT_TYPE InteractType);

	void OnDisInteractable();

	UFUNCTION(BlueprintCallable, Category = "Interacter|Interact")
	bool TryInteractInput(bool& OutIsInterupt, bool& OutIsInteractEnter);
};
