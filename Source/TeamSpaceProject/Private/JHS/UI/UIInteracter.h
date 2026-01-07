// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "JHS/UI/UIBase.h"

#include "UIInteracter.generated.h"

class UUIManager;
class AJHSPlayerBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UUIInteracter : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIInteracter();

private:
	TObjectPtr<UUIManager> _uiManager = nullptr;

	TObjectPtr<UBoxComponent> _collisionComponent = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|Open UI")
	E_UI_TYPE _openUIType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIInteracter|Collision")
	FVector _collisionBoxExtent = FVector(100.0f, 100.0f, 100.0f);

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
	void OpenUI();

	void CloseUI();
};
