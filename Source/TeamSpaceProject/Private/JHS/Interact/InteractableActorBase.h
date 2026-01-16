// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/CommonEnums.h"

#include "InteractableActorBase.generated.h"

UCLASS()
class AInteractableActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActorBase();

private:
	TObjectPtr<UInteractableComponent> _uiInteracterable = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChairBase|Debug")
	bool _isDebugDraw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChairBase|UIInteractable")
	float _interactRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChairBase|UIInteractable")
	E_UI_TYPE _interatUIType;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void InteractEnter();

	UFUNCTION()
	void InteractExit();

protected:
	virtual void OnInteractEnter() { }
	
	virtual void OnInteractExit() { }

};
