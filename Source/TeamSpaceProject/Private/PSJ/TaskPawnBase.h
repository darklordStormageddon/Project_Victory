// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TaskPawnBase.generated.h"

UCLASS()
class ATaskPawnBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATaskPawnBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void SetPilot(ACharacter* Character);

	/** 탑승 성공 시 클라이언트에서 입력/UI 등 역할별 설정. 서버에서 Possess 직후 호출. */
	UFUNCTION(Client, Reliable)
	void Client_BoardingSuccess();

protected:
	virtual void Client_BoardingSuccess_Implementation();
};
