// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Asteroid/AsteroidComponent.h"

#include "TestSpaceShip.generated.h"

UCLASS()
class ATestSpaceShip : public ACharacter
{
	GENERATED_BODY()
private:

	UPROPERTY(VisibleAnywhere)
	UAsteroidComponent* AsteroidComponent;
public:	
	ATestSpaceShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
