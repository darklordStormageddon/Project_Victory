// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AS_ConnectionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAS_ConnectionComponent : public USceneComponent
{
	GENERATED_BODY()

private:
	FTimerHandle AskTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Ask")
	float AskDelayTime = 0.2f;

	bool DelayAsk = true;

	UPROPERTY(VisibleAnywhere, Category = "Connection")
	UStaticMeshComponent* ConnectionMesh;

	UPROPERTY(VisibleAnywhere, Category = "Connection")
	UStaticMeshComponent* OutlineMesh;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Connection")
	bool CanSeparate;

private:
	void HaveChild();

public:
	// Sets default values for this component's properties
	UAS_ConnectionComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CanAsk();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void F_CanSeparate();

		
};
