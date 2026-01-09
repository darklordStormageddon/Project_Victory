// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableBase.h"
#include "Components/SphereComponent.h"
#include "JHS/UI/UIBase.h"

#include "UIInteracterable.generated.h"

class UUIManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UUIInteracterable : public UInteractableBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIInteracterable();

private:
	TObjectPtr<UUIManager> _uiManager = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interacter|Type")
	E_UI_TYPE _openUIType;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

public:
	
};
