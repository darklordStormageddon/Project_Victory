// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"

#include "TaskChair.generated.h"

class APawn;
class UUIBase;

UCLASS()
class ATaskChair : public AInteractableActorBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Replicated, Category = "Link")
	TObjectPtr<APawn> TargetTaskPawn = nullptr;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI) override;

public:
	virtual void Tick(float DeltaTime) override;
};
