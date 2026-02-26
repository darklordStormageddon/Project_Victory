// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/UI/UIBase.h"

#include "InteracterComponent.generated.h"

class UUIManager;
class UInteractableComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInteracterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteracterComponent();

private:
	UPROPERTY()
	APawn* _ownerPawn = nullptr;

	const E_UI_TYPE _playerUI = E_UI_TYPE::UIPanelPlayerFPS;

	UPROPERTY()
	TObjectPtr<APlayerController> _playerController = nullptr;

	UPROPERTY()
	TObjectPtr<UUIManager> _uiManager = nullptr;

	UPROPERTY()
	TObjectPtr<UInteractableComponent> _interactable = nullptr;

public:
	TObjectPtr<APlayerController> GetPlayerController() { return _playerController; }

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

	// Pawn 소유이므로 owning connection 있음 → 클라이언트에서 서버로 트리거 알림 (Interactable은 레벨 액터라 RPC 불가)
	UFUNCTION(Server, Reliable)
	void ServerReportTriggerEnter(UInteractableComponent* Interactable);

	UFUNCTION(Server, Reliable)
	void ServerReportTriggerExit(UInteractableComponent* Interactable);

private:
	void ExecuteEventOnChangeInteractType(E_INTERACT_TYPE InteractType);
};
