#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
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

	// 모든 자식이 공유할 하차 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Interact;

	void Input_Exit(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Interaction")
	void Server_RequestDisembark();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Pilot")
	APSJ_Character* CurrentPilot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Points")
	UArrowComponent* RidePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Points")
	UArrowComponent* ExitPoint;

	UPROPERTY(VisibleInstanceOnly, Category = "Connection")
	AInteractableActorBase* LinkedSeat = nullptr;

public:
	virtual void SetPilot(ACharacter* Character);

	UFUNCTION(Client, Reliable)
	void Client_BoardingSuccess();
	virtual void Client_BoardingSuccess_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void DisembarkCharacter();

	UFUNCTION(Client, Reliable)
	void Client_DisembarkSuccess(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot);
};