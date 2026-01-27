// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "CollectStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCollectStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCollectStateGroup();

private:
	const float CONSUME_DURABILITY = 1.0f;

	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY()
	TMap<E_COLLECT_TOOL_TYPE, FCollectToolData> _collectToolDataMap;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeCollectState(TObjectPtr<AJHSGameState> GameState);

	void UpdateCollectState();

	void RepairAllTool();

	bool TryUseTool(E_COLLECT_TOOL_TYPE CollectToolType, float& OutToolDamage);

private:
	void LoadCollectToolDataTable();

	bool TryGetCollectToolData(E_COLLECT_TOOL_TYPE CollectToolType, FCollectToolData*& OutCollectToolData);

	void ExecuteEventTool(FCollectToolData CollectToolData);
};
