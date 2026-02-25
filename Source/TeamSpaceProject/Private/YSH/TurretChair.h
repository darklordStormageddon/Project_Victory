// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PSJ/TaskChair.h"
#include "TurretChair.generated.h"

UCLASS()
class ATurretChair : public ATaskChair
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATurretChair();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void SetTurretPawn(TObjectPtr<APawn> TurretPawn);
};
