// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "ContainerStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UContainerStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UContainerStateGroup();

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY()
	FContainerState _containerState;

	/** 리플리케이트용. 서버에서 _containerState 갱신 시 이 배열들 동기화. */
	UPROPERTY(ReplicatedUsing = "OnRep_ContainerStateReplicated")
	TArray<FElementData> _replicatedElementArray;

	UPROPERTY(ReplicatedUsing = "OnRep_OwnedDollar")
	int32 _replicatedOwnedDollar = 0;

	UPROPERTY(ReplicatedUsing = "OnRep_GoalDollar")
	int32 _replicatedCurrentGoalDollar = 0;

	UPROPERTY(Replicated)
	float _replicatedSaleInterval = 0.5f;

	FTimerHandle _saleAllElementTimerHandle;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	int32 GetOwnedDollar() { return _containerState.OwnedDollar; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeContainerState(TObjectPtr<AJHSGameState> GameState, FContainerState InitContainerState);

	void UpdateContainerState();

	void SetStageGoalDollar(int32 Stage);

	void AddElement(E_ELEMENT_TYPE ElementType, int32 Amount);

	void SaleAllElement();

	void ServerSaleAllElement();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSaleElementInternal(int32 ElementTypeIndex);

private:
	void SaleElementInternal(int32 ElementTypeIndex);

public:
	/** 보유 달러에서 차감 시도. 성공 시 true 및 차감, 실패 시 false. */
	bool TryConsumeDollar(int32 Amount);

	bool TryGetElementData(E_ELEMENT_TYPE ElementType, FElementData*& OutElementData);

private:
	void LoadElementData();

	UFUNCTION()
	void OnRep_ContainerStateReplicated();

	UFUNCTION()
	void OnRep_OwnedDollar();

	UFUNCTION()
	void OnRep_GoalDollar();

	void ExecuteEventOnChangeElement(FElementData ElementData);

	void ExecuteEventOnChangeOwnedDollar(int32 OwnedDollar);

	void ExecuteEventOnChangeGoalDollar(int32 GoalDollar);

	void SyncContainerStateToReplicated();
};
