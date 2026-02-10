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
	UPROPERTY()
	TObjectPtr<UInteractableComponent> _interacterable = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Debug")
	bool _isDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|ActorBase")
	float _interactRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|ActorBase")
	E_UI_TYPE _interatUIType = E_UI_TYPE::NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|ActorBase")
	bool _isWorldSpaceUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|ActorBase")
	FVector _worldUIRelativeLocation = FVector(0, 0, 150);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|ActorBase")
	float _worldUIScale = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void InteractEnter(AActor* Caller, UUIBase* OpenedUI);

	UFUNCTION()
	void InteractExit(AActor* Caller, UUIBase* ClosedUI);

protected:
	virtual void OnInteractEnter(AActor* Caller, TObjectPtr<UUIBase> OpenedUI) { }
	
	virtual void OnInteractExit(AActor* Caller, TObjectPtr<UUIBase> ClosedUI) { }

};
