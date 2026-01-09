// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/CommonEnums.h"

#include "InteractableBase.generated.h"

class UUIInteracter;
class UUIManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractEnterAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractExitAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractInterruptAction);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInteractableBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableBase();

private:
	TObjectPtr<USphereComponent> _collisionComponent = nullptr;

	TObjectPtr<UUIInteracter> _interacter = nullptr;

	bool _isInteract = false;

	TObjectPtr<UUIInteracter> _InterruptInteracter = nullptr;

	TObjectPtr<UUIManager> _uiManager = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Debug")
	bool _isDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float _collisionRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	E_INTERACT_TYPE _interactType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool _isInterrupt = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	E_UI_TYPE _interactUIType;

public:
	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractEnterAction OnInteractEnterAction;

	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractExitAction OnInteractExitAction;

	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractInterruptAction OnInteractInterruptAction;

public:
	E_INTERACT_TYPE GetInteractType() { return _interactType; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	void InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_INTERACT_TYPE InteractType, E_UI_TYPE InteractUIType);

	bool TryInteract(bool& OutIsInterupt, bool& OutIsInteractEnter);

private:
	void ChangeInteractState(bool IsInteract);
};
