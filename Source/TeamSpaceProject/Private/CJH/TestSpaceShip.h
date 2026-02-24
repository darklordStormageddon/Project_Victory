// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PSJ/PSJ_Spaceship.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TestSpaceShip.generated.h"

UCLASS()
class ATestSpaceShip : public APSJ_Spaceship
{
	GENERATED_BODY()
private:
	FTimerHandle ShieldAlphaTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float ShieldDisplayDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	USceneComponent* ShieldRoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Shield")
	UStaticMeshComponent* ShieldMesh = nullptr;

private:
	void HideShield();

public:	
	// Sets default values for this actor's properties
	ATestSpaceShip();

	void OnTakeDamage(float Damage);
	//virtual void OnTakeDamage(float Damage) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
