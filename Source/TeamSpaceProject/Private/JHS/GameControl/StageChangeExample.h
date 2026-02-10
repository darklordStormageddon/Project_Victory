// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/Event/CommonEventBase.h"
#include "StageChangeExample.generated.h"

UCLASS()
class AStageChangeExample : public AActor
{
	GENERATED_BODY()

private:
	FDelegateHandle _eventHandleOnStartStage;

	FDelegateHandle _eventHandleOnEndStage;

public:
	AStageChangeExample();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void OnStartStage(UEventOnStartStage* Event);

	void OnEndStage(UEventOnEndStage* Event);
};
