#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "TaskPawnBase.generated.h"


class APSJ_Character;
class UArrowComponent;
class AInteractableActorBase;

UCLASS()
class ATaskPawnBase : public APawn
{
	GENERATED_BODY()

public:
	ATaskPawnBase();

private:
	FDelegateHandle _eventHandleOnEndStage;
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Interact;

	void Input_Exit(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Interaction")
	void Server_RequestDisembark();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Pilot")
	APSJ_Character* CurrentPilot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Points")
	FVector DisembarkOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(VisibleInstanceOnly, Category = "Connection")
	AInteractableActorBase* LinkedSeat = nullptr;

public:
	virtual void SetPilot(ACharacter* Character);


	UFUNCTION(Client, Reliable)
	void Client_BoardingSuccess(APSJ_Character* BoardingPilot);

	virtual void Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void DisembarkCharacter();

	UFUNCTION(Client, Reliable)
	void Client_DisembarkSuccess(APSJ_Character* ExitingPilot, FVector LocalLoc, FRotator LocalRot);

	virtual void Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector LocalLoc, FRotator LocalRot);

	void OnEndStage(UEventOnEndStage* Event);
};