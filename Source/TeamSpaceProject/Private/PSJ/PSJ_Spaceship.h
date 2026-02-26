#pragma once

#include "CoreMinimal.h"
#include "PSJ/TaskPawnBase.h"
#include "InputActionValue.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PSJ_Spaceship.generated.h"

class USphereComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

class APSJ_Character;
class ATaskChair;

class UHealthComponent;
class USpaceShipStateGroup;
class AJHSGameState;

class UDistanceComponent;

UCLASS()
class TEAMSPACEPROJECT_API APSJ_Spaceship : public ATaskPawnBase
{
	GENERATED_BODY()

private:

	FTimerHandle ShieldAlphaTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float ShieldDisplayDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	USceneComponent* ShieldRoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Shield")
	UStaticMeshComponent* ShieldMesh = nullptr;


	UPROPERTY(VisibleAnywhere, Category = "DistanceComp")
	UDistanceComponent* DistanceComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "DistanceDamage")
	float OverDistanceDamage = 1.0f;

private:
	AJHSGameState* _outGameState = nullptr;

private:
	void ShowShield();
	void HideShield();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ShowShield();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_HideShield();

	UFUNCTION()
	void OverDistanceDamageCheck();

public:
	APSJ_Spaceship();

	bool TryMove();

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ThrustForward(float Value);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ThrustBackward(float Value);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_MoveAxes(FVector2D Value);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_MoveUp(float Value);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_Roll(float Value);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_MouseLook(FVector2D Value);

protected:
	virtual void BeginPlay() override;

	virtual void Client_BoardingSuccess_Implementation() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UPrimitiveComponent* ShipRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	UCameraComponent* PilotCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship Components")
	USphereComponent* PilotSphere;


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


	UPROPERTY(EditAnywhere, Category = "Ship Stats")
	float ThrustSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Ship Stats")
	float RotateSpeed = 1.0f;


	UPROPERTY(EditAnywhere, Category = "Ship Stats")
	float MaxSpeed = 4000.0f;




protected:


	FTimerHandle CollisionResetTimerHandle;

	USpaceShipStateGroup* _spaceShipStateGroup = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthComp;

public:

	void Input_ThrustForward(const FInputActionValue& Value);
	void Input_ThrustBackward(const FInputActionValue& Value);
	void Input_MoveAxes(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MouseLook(const FInputActionValue& Value);
	void Input_Roll(const FInputActionValue& Value);




	void EnableCollisionWithPassenger(APSJ_Character* ExitedChar);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UPROPERTY(VisibleInstanceOnly, Category = "Connection")
	ATaskChair* LinkedChair;

	UFUNCTION()
	void OnTakeDamage(float Damage);

	UFUNCTION()
	void OnDeath();

	USpaceShipStateGroup* GetSpaceShipStateGroup();
};