// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"
#include "YSHPlayerBase.generated.h"

class AActor;
class UUIInteracterable;

UCLASS()
class AYSHPlayerBase : public ATeamSpaceProjectCharacter
{
	GENERATED_BODY()

public:
	AYSHPlayerBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// UIInteracterable에서 진입/해제 시 호출해줄 API
	void EnterInteractable(UUIInteracterable* Interactable, AActor* NewViewTarget);
	void ExitInteractable();

	bool IsInteracting() const { return bIsInteracting; }

private:
	void SetPlayerControlLock(bool bLock);
	void SwitchToViewTarget(AActor* NewViewTarget, float BlendTime = 0.2f);
	void RestoreViewTarget(float BlendTime = 0.2f);

private:
	UPROPERTY(Transient)
	TObjectPtr<UUIInteracterable> CurrentInteractable = nullptr;

	UPROPERTY(Transient)
	bool bIsInteracting = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SavedViewTarget;

	UPROPERTY(Transient)
	bool bHasSavedViewTarget = false;
};