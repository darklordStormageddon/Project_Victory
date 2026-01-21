// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SphereComponent.h"
#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"

#include "TestSpaceShip.generated.h"


class UHealthComponent;
class USpaceShipStateGroup;

UCLASS()
class ATestSpaceShip : public ATeamSpaceProjectCharacter
{
	GENERATED_BODY()

protected:
	USpaceShipStateGroup* _spaceShipStateGroup = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	USphereComponent* Collision;

public:	
	// Sets default values for this actor's properties
	ATestSpaceShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTakeDamage(float Damage);

	UFUNCTION()
	void OnDeath();

	USpaceShipStateGroup* GetSpaceShipStateGroup();
};
