// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIManager.generated.h"

UENUM(BlueprintType)
enum class E_UI_TYPE : uint8
{
	// Panel
	UIPanelPlayer = 0 UMETA(DisplayName = "UIPanelPlayer"),
	UIPanelDriveSeat UMETA(DisplayName = "UIPanelDriveSeat"),
	UIPanel UMETA(DisplayName = "SpaceGarbage"),

	// Popup
	UIPopupCommon = 100 UMETA(DisplayName = "UIPopupCommon"),

	// System
	UISystemSetting = 200 UMETA(DisplayName = "UISystemSetting"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UUIManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void OpenUI(E_UI_TYPE UIType);

	void CloseUI(E_UI_TYPE UIType);
};
