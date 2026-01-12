#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h" // FInputActionValue 구조체 사용을 위해 필수
#include "SpaceShip.generated.h" // 반드시 마지막에 위치

// 전방 선언: 헤더에서는 이름만 알려주고 실제 상세 내용은 소스 파일에서 포함합니다.
class USphereComponent;
class UStaticMeshComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class TEAMSPACEPROJECT_API ASpaceShip : public APawn
{
    GENERATED_BODY()

public:
    ASpaceShip();

protected:
    virtual void BeginPlay() override; // 엔진 기본 함수이므로 override 사용

public:
    virtual void Tick(float DeltaTime) override; // 엔진 기본 함수이므로 override 사용
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // --- 컴포넌트 섹션 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* RootCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* FirstPersonCamera;

    // --- 비행 파라미터 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShipStats")
    float ThrustStrength = 1000000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShipStats")
    float MaxSpeed = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShipStats")
    float RotationSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShipStats")
    float StoppingThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShipStats")
    float StoppingDamping = 2.0f;

    // --- 향상된 입력 에셋 연결 변수 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputMappingContext* ShipMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputAction* IA_ThrustForward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputAction* IA_ThrustBackward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputAction* IA_MoveAxes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputAction* IA_Roll;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShipControls")
    UInputAction* IA_MouseLook;

private:
    float ForwardInput = 0.0f;
    FVector RotationInput = FVector::ZeroVector;
    FVector StrafeInput = FVector::ZeroVector;

    void ApplyFlightPhysics(float DeltaTime);
    void HandleVelocityClamp();

    // 사용자 정의 입력 함수 (부모 클래스에 없는 함수이므로 override를 붙이지 않습니다)
    void OnThrustForward(const FInputActionValue& Value);
    void OnThrustBackward(const FInputActionValue& Value);
    void OnMoveAxes(const FInputActionValue& Value);
    void OnRoll(const FInputActionValue& Value);
    void OnMouseLook(const FInputActionValue& Value);
};