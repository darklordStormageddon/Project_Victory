// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Asteroid/AsteroidComponent.h"

#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"

#include "TestCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ATestCharacter : public ATeamSpaceProjectCharacter
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Speed")
		float Speed;

	UPROPERTY(VisibleAnywhere)
		UAsteroidComponent* AsteroidComponent;
public:
	ATestCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
