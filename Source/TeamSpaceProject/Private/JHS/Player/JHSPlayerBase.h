// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"

#include "JHSPlayerBase.generated.h"

class AJHSGameState;

UCLASS()
class AJHSPlayerBase : public ATeamSpaceProjectCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AJHSPlayerBase();

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// Text Code
private:
	FTimerHandle _timerHandle;

	int32 _elementIndex = -1;

	int32 _toolIndex = -1;

private:
	void FireTurret();

	void AddElement();

	void UseCollectTool();
};
