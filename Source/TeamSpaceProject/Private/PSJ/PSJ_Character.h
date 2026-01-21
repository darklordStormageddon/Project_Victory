#pragma once
#define ECC_Spaceship_Floor ECC_GameTraceChannel5
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PSJ_Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class APawn;

UCLASS()
class TEAMSPACEPROJECT_API APSJ_Character : public ACharacter
{
	GENERATED_BODY()
public:
	APSJ_Character();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	APawn* CurrentSpaceship = nullptr;
	void SetCurrentSpaceship(APawn* NewSpaceship);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;
	void Interact(const FInputActionValue& Value);
protected:
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float CheckDistance = 400.0f;
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float MagBootsTraceRadius = 25.0f;
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float SpringStiffness = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float SpringDamping = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float AlignSpeed = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Movement Stats")
	float FlyModeMaxSpeed = 600.0f;
	UPROPERTY(EditAnywhere, Category = "Movement Stats")
	float FlyModeBrakingDeceleration = 10000.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mag Boots")
	bool bIsMagBootsActive = false;
	UPROPERTY(Transient)
	AActor* LastFloorActor = nullptr;
	FVector CurrentFloorNormal = FVector::UpVector;
	float DefaultMeshZ = 0.0f;

	// [추가] 거리 기반 댐핑 계산을 위한 변수
	float LastFloorDistance = 0.0f;

	void UpdateMagBoots(float DeltaTime);




protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	UCameraComponent* FPSCamera;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};