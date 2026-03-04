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

	UFUNCTION(BlueprintCallable, Category = "PlayerController|UI Manager")
	UUIManager* GetUIManager() { return _uiManager; }

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void SetAssignedPlayerId(int32 AssignedPlayerId);

	/** 트리거 진입 시 해당 클라이언트만 호출. 복제 타이밍에 의존하지 않음. */
	UFUNCTION(Client, Reliable)
	void ClientInteractableTriggerEnter(UInteractableComponent* Interactable, E_INTERACT_TYPE InteractType);

	/** 트리거 이탈 시 해당 클라이언트만 호출. 복제 타이밍에 의존하지 않음. */
	UFUNCTION(Client, Reliable)
	void ClientInteractableTriggerExit(UInteractableComponent* Interactable);

	/** 월드 UI 토글 요청. 서버에서만 상태 갱신 후 멀티캐스트로 결과 전파 (Interactable은 owning connection 없어 컨트롤러 경유). */
	UFUNCTION(Server, Reliable)
	void ServerRequestToggleWorldUI(UInteractableComponent* Target);

	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseSpaceShip(E_SPACE_SHIP_DATA_TYPE DataType);

	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseCollectTool(E_COLLECT_TOOL_TYPE ToolType, bool IsDurability);

	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 FieldIndex);

	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseAmmo(E_AMMO_TYPE AmmoType, int32 FieldIndex);

	UFUNCTION(Server, Reliable)
	void ServerRequestSaleAllElement();

	UFUNCTION(Server, Reliable)
	void ServerRequestEndStage();
};
