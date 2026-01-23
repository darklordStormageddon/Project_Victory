// [PSJ_Character.h]

#pragma once
#define ECC_Spaceship_Floor ECC_GameTraceChannel5
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h" // 리플리케이션 필수 헤더
#include "PSJ_Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class APawn;

// [필수 구조체] 상대 좌표 동기화용 데이터
USTRUCT()
struct FRelativeSpaceData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize100 RelativeLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY()
	AActor* BaseActor = nullptr;

	UPROPERTY()
	bool bIsAnchored = false;
};

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
	// [추가] 컨트롤러가 빙의할 때 상태 리셋을 위한 함수
	virtual void PossessedBy(AController* NewController) override;
	// [추가됨] 화면 떨림 방지를 위한 카메라 보정 함수
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	// [추가됨] 리플리케이션 설정 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	float AlignSpeed = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Stats")
	float FlyModeMaxSpeed = 600.0f;

	// [추가됨] WASD 입력을 저장할 변수 (Tick에서 사용)
	FVector2D CurrentInputVector = FVector2D::ZeroVector;

	// [추가됨] 서버와 동기화할 상대 좌표 데이터
	UPROPERTY(Replicated)
	FRelativeSpaceData ReplicatedRelativeData;

	// 바닥 감지 및 로컬 보정 함수
	void UpdateMagBoots(float DeltaTime);

	// [추가됨] 서버로 상대 위치를 업데이트하는 RPC 함수
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_UpdateRelativeTransform(FVector NewRelLoc, FRotator NewRelRot);

	// 기존 변수들 (유지)
	FVector CurrentFloorNormal = FVector::UpVector;
	UPROPERTY(Transient)
	AActor* LastFloorActor = nullptr;
	float DefaultMeshZ = 0.0f;

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
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};