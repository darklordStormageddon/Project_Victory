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

	FTimerHandle _saleAllElementTimerHandle;

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

	void AddElement(E_ELEMENT_TYPE ElementType, int32 Amount);

	void SaleAllElement();
	
	bool TryGetElementData(E_ELEMENT_TYPE ElementType, FElementData*& OutElementData);

private:
	void LoadElementData();

	void SaleElementInternal(int32 ElementTypeIndex);

	void ExecuteEventOnChangeElement(FElementData ElementData);
};
