// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KSM/HealthComponent.h"

#include "TargetBase.generated.h"

USTRUCT()
struct FTargetInfo
{
	GENERATED_BODY()

	float Size;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Max_HP;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Attack_Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float Speed;
};

UCLASS()
class ATargetBase : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	FTargetInfo _targetInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;

public:	
	// Sets default values for this actor's properties
	ATargetBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
