// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "JHS/UI/UIBase.h"

#include "UIInteracterable.generated.h"

class UUIManager;
class UUIInteracter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UUIInteracterable : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIInteracterable();

private:
	TObjectPtr<UUIManager> _uiManager = nullptr;

	TObjectPtr<USphereComponent> _collisionComponent = nullptr;

	TObjectPtr<UUIInteracter> _uiInteracter = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|Debug")
	bool _isDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|Collision")
	float _collisionRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|UI")
	E_UI_TYPE _openUIType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|UI")
	bool _isInteract = false;

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
	bool TryInteract();

private:
	void OpenUI();

	void CloseUI();
};
