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

class UHealthComponent;
class USpaceShipStateGroup;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UArrowComponent* RidePoint;

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

	// [추가] 현재 조종사를 반환하는 Getter 함수
	UFUNCTION(BlueprintPure, Category = "Pilot")
	APSJ_Character* GetCurrentPilot() const { return CurrentPilot; }

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pilot")
	APSJ_Character* CurrentPilot = nullptr;

	FTimerHandle CollisionResetTimerHandle;



	USpaceShipStateGroup* _spaceShipStateGroup = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;


public:
	void SetPilot(APSJ_Character* NewPilot);

	void Input_ThrustForward(const FInputActionValue& Value);
	void Input_ThrustBackward(const FInputActionValue& Value);
	void Input_MoveAxes(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MouseLook(const FInputActionValue& Value);
	void Input_Roll(const FInputActionValue& Value);
	void Input_Exit(const FInputActionValue& Value);

	// [추가] 탑승 성공 시 클라이언트에게 설정(입력, UI)을 지시하는 RPC
	UFUNCTION(Client, Reliable)
	void Client_BoardingSuccess();

	// [추가] 하차 성공 시 클라이언트 설정을 정리하는 RPC
	UFUNCTION(Client, Reliable)
	void Client_DisembarkSuccess();

	// [추가] 서버에 하차를 요청하는 RPC 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDisembark();

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

	UFUNCTION()
	void OnTakeDamage(float Damage);

	UFUNCTION()
	void OnDeath();

	USpaceShipStateGroup* GetSpaceShipStateGroup();



};