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

	// === [BP 컴포넌트 참조용 포인터] ===
	// C++에서 생성하지 않고, BP에 있는 것을 찾아와서 담을 변수들입니다.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UPrimitiveComponent* ShipRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UCameraComponent* PilotCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	USphereComponent* PilotSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UArrowComponent* ExitPoint;

	// === [입력 액션] ===
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

	// === [설정값] ===
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

	// === [조작 함수] ===
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
};