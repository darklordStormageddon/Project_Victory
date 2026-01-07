// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AS_Connection.generated.h"

UCLASS()
class AAS_Connection : public AActor
{
	GENERATED_BODY()
private:
	FTimerHandle AskTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Ask")
	float AskDelayTime = 0.2f;

	bool DelayAsk = true;

	UPROPERTY(VisibleAnywhere, Category = "Wing")
	UStaticMeshComponent* WingMesh;

	UPROPERTY(VisibleAnywhere, Category = "Wing")
	UStaticMeshComponent* OutlineMesh;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Connection")
	bool CanSeparate;

public:	
	// Sets default values for this actor's properties
	AAS_Connection();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CanAsk();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Connection")
	void HaveChild();

	void F_CanSeparate();
};
