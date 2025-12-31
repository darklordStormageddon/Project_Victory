// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/SpaceObject/SpaceObjectBase.h"
#include "TestSpaceObject.generated.h"

UCLASS()
class ATestSpaceObject : public ASpaceObjectBase
{
	GENERATED_BODY()
	
private:
	float _currentMovementTime = 0.0f;
	bool _isMovingToEnd = true;
	float _accumulatedYaw = 0.0f;

	FVector _startMovementLocation;
	FRotator _startMovementRotator;
	FVector _endMovementLocation;
	FRotator _endMovementRotator;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RepeatMove")
	TObjectPtr<ASpaceObjectBase> StartPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RepeatMove")
	TObjectPtr<ASpaceObjectBase> EndPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RepeatMove")
	float MovementDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RepeatMove")
	float ReachDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RepeatMove")
	float YawRotationSpeed = 0.0f;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void MovementTick(float DeltaTime) override;

private:
	void StartMovement();
};
