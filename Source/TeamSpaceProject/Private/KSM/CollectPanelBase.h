// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "CollectPanelBase.generated.h"

class UCollectStateGroup;

UCLASS()
class ACollectPanelBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollectPanelBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY()
	TObjectPtr<UCollectStateGroup> _collectStateGroup;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void UpdateDurabilityAndDamage();

public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	E_COLLECT_TOOL_TYPE _Tool_Type;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	float _OutToolDamage = 0.0f;
};
