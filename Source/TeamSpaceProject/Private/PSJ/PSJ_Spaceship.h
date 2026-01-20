#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Components/ArrowComponent.h"
#include "PSJ_Spaceship.generated.h"

class USphereComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class APSJ_Character;
class APSJ_ShipCockpit;

UCLASS()
class TEAMSPACEPROJECT_API APSJ_Spaceship : public APawn
{
	GENERATED_BODY()

public:
	APSJ_Spaceship();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UPrimitiveComponent* ShipRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UCameraComponent* PilotCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	USphereComponent* PilotSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UArrowComponent* ExitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* ShipMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ThrustForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ThrustBackward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_MoveAxes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_MoveUp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_MouseLook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Interact;

	UPROPERTY(EditAnywhere, Category = "Ship Stats")
	float ThrustSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Ship Stats")
	float RotateSpeed = 1.0f;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pilot")
	APSJ_Character* CurrentPilot = nullptr;

	FTimerHandle CollisionResetTimerHandle;

public:
	void SetPilot(APSJ_Character* NewPilot);

	void Input_ThrustForward(const FInputActionValue& Value);
	void Input_ThrustBackward(const FInputActionValue& Value);
	void Input_MoveAxes(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MouseLook(const FInputActionValue& Value);
	void Input_Roll(const FInputActionValue& Value);
	void Input_Exit(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void DisembarkCharacter();

	void EnableCollisionWithPassenger(APSJ_Character* ExitedChar);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// [추가] 현재 연결된 조종석 (내릴 때 UI 끄기용)
	UPROPERTY(VisibleInstanceOnly, Category = "Connection")
	APSJ_ShipCockpit* LinkedCockpit;

};