// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JHS/GameControl/CommonEnums.h"

#include "JHSPlayerController.generated.h"

class UUIManager;
class UInteractableComponent;

UCLASS()
class AJHSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AJHSPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "PlayerController|Manager")
	TObjectPtr<UUIManager> _uiManager = nullptr;

	/** 서버 PostLogin 시 PlayerStateGroup에서 발급·복제된 실별 ID. UI/Interact 호출자 비교용. */
	UPROPERTY(Replicated)
	int32 _assignedPlayerId = -1;

public:
	/** 서버에서 발급·복제된 AssignedPlayerId (PlayerStateGroup과 동일값). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	int32 GetAssignedPlayerId() const { return _assignedPlayerId; }

	void SetAssignedPlayerId(int32 AssignedPlayerId);

	UFUNCTION(BlueprintCallable, Category = "PlayerController|UI Manager")
	UUIManager* GetUIManager() { return _uiManager; }

	/** 트리거 진입 시 해당 클라이언트만 호출. 복제 타이밍에 의존하지 않음. */
	UFUNCTION(Client, Reliable)
	void ClientInteractableTriggerEnter(UInteractableComponent* Interactable, E_INTERACT_TYPE InteractType);

	/** 트리거 이탈 시 해당 클라이언트만 호출. 복제 타이밍에 의존하지 않음. */
	UFUNCTION(Client, Reliable)
	void ClientInteractableTriggerExit(UInteractableComponent* Interactable);
};
