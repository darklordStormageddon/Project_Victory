// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define ECC_Spaceship_Floor ECC_GameTraceChannel5
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PSJ_Character.generated.h"

// 전방 선언
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

	// === [상호작용: 우주선 탑승] ===
public:
	// 현재 탑승 가능한 우주선 (Overlap 되면 이 변수에 저장됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	APawn* CurrentSpaceship = nullptr;

	// 우주선이 호출해줄 함수
	void SetCurrentSpaceship(APawn* NewSpaceship);

protected:
	// 상호작용(F키) 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

	// 상호작용 실행 함수
	void Interact(const FInputActionValue& Value);


	// === [자석 신발 (Mag Boots)] ===
protected:
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float CheckDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float SpringStiffness = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float SpringDamping = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float AlignSpeed = 15.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mag Boots")
	bool bIsMagBootsActive = false;

	UPROPERTY(Transient)
	AActor* LastFloorActor = nullptr;

	FVector CurrentFloorNormal = FVector::UpVector;

	float DefaultMeshZ = 0.0f;

	void UpdateMagBoots(float DeltaTime);


	// === [입력 및 카메라] ===
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