// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Connector_Base.generated.h"

class ASatellite_Base;
class UHealthComponent;

UCLASS()
class AConnector_Base : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AConnector_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Root")
	ASatellite_Base* Attached_Satellite;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Damage_Connector(float Damage);
};
