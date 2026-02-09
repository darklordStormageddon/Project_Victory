// [PSJ_Character.h]

#pragma once
#define ECC_Spaceship_Floor ECC_GameTraceChannel5
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h" 
#include "Engine/NetSerialization.h" // [추가] 이것이 있어야 NetQuantize100 사용 가능
#include "PSJ_Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class APawn;
class APSJ_Spaceship;
// 전방 선언 추가
class ATurretBase_GT;

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

	void Client_RestoreInput();

	virtual void PossessedBy(AController* NewController) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	void ForceClearAnchoring();
	void Client_ForceCleanupImmediate();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



private:
	// 하차 직후 상태 관리를 위한 변수 추가
	bool bJustDisembarked = false;
	float DisembarkGraceTimer = 0.0f;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	APawn* CurrentSpaceship = nullptr;
	void SetCurrentSpaceship(APawn* NewSpaceship);

	// [중요] 위에서 class APSJ_Spaceship; 을 선언했기 때문에 이제 에러가 나지 않습니다.BlueprintCallable 추가!
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_RequestBoarding(APSJ_Spaceship* ShipToBoard);

	// ▼ [신규 추가] 범용 탑승 요청 (패널, 의자 등 아무 폰이나 조종 요청)
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_RequestPawnPossess(APawn* TargetPawn);

	// 하차 상태 시작 함수
	void StartDisembarkState();

	// [신규] 터렛 탑승 요청 추가
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestTurretBoarding(ATurretBase_GT* TurretToBoard, APSJ_ShipCockpit* LinkedCockpit);

	// [신규] 타이머를 통해 호출될 최종 입력 복구 함수
	void Client_LateInputRestore();

	// [추가] 하차 시 서버/클라 양쪽에서 변수를 세팅할 함수
	void SetBaseActorData(AActor* NewBase);

	// [신규] 하차 시 강제로 입력을 활성화하는 함수
	void ForceInputRecovery();

	// [신규] 입력 바인딩용 함수
	void Input_ForceEject(const FInputActionValue& Value);

	// [신규] 서버 RPC 함수 선언
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryForceEject(APSJ_ShipCockpit* TargetCockpit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;
	void Interact(const FInputActionValue& Value);

	// [추가] 클라이언트 전용: "서버가 날 조종하라고 보낸 컨트롤러가 도착했다!" 라는 검증 함수
	virtual void OnRep_Controller() override;

	// --- 벽 감지용 설정 변수 (에디터 수정 가능) ---
	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	float WallTraceRadius = 40.0f; // 구체 트레이스 반지름 (지름의 절반)

	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	float WallTraceZOffset = 0.0f; // 캐릭터 중심 기준 Z축 오프셋

	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	bool bShowWallDebug = true; // 디버그 라인 표시 여부

protected:

	// [신규] 클라이언트가 바닥을 감지하면 서버에 "나 여기 붙여줘"라고 요청하는 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetAnchoring(AActor* NewBase);

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float CheckDistance = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float MagBootsTraceRadius = 15.0f;

	// [신규] 길이(Half Height) 추가 - 에디터 수정 가능
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float MagBootsTraceHalfHeight = 30.0f;

	// [추가] 바닥에서 캐릭터 캡슐 하단을 얼마나 띄울지 결정하는 오프셋 (기존 2.0f)
	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float FloorHeightOffset = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float AlignSpeed = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Stats")
	float FlyModeMaxSpeed = 600.0f;

	FVector2D CurrentInputVector = FVector2D::ZeroVector;

	UPROPERTY(Replicated)
	FRelativeSpaceData ReplicatedRelativeData;

	void UpdateMagBoots(float DeltaTime);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_UpdateRelativeTransform(FVector NewRelLoc, FRotator NewRelRot);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	void Input_Jump(const FInputActionValue& Value);

	// [필수 추가 1] 강제 하차 입력 액션 (에디터 할당 필요)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ForceEjectAction;

	// [필수 추가 2] 감지 거리 (기본값 500.0f)
	UPROPERTY(EditAnywhere, Category = "Interaction | Force Eject")
	float ForceEjectRange = 500.0f;

	// [수정] 중력(Gravity) 용어 제거 -> 자력에 의한 감속(Deceleration)으로 변경
	UPROPERTY(EditAnywhere, Category = "Mag Boots | Jump")
	float JumpInitialSpeed = 450.0f; // 초기 도약 속도

	UPROPERTY(EditAnywhere, Category = "Mag Boots | Jump")
	float JumpDeceleration = 980.0f; // 자석이 당기는 힘 (감속도)

	// 점프 상태 변수
	bool bIsJumping = false;
	float CurrentVerticalSpeed = 0.0f;

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};